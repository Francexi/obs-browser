/******************************************************************************
 Copyright (C) 2014 by John R. Bradley <jrb@turrettech.com>
 Copyright (C) 2018 by Hugh Bailey ("Jim") <jim@obsproject.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "obs-browser-source.hpp"
#include "browser-client.hpp"
#include "browser-scheme.hpp"
#include "wide-string.hpp"
#include "json11/json11.hpp"
#include <util/threading.h>
#include <QApplication>
#include <util/dstr.h>
#include <functional>
#include <thread>
#include <mutex>

#ifdef __LINUX__
#define XK_3270 // for XK_3270_BackTab
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/Xcursor/Xcursor.h>
#include "windows-keycode.h"
#endif

#ifdef USE_QT_LOOP
#include <QEventLoop>
#include <QThread>
#endif

using namespace std;
using namespace json11;

extern bool QueueCEFTask(std::function<void()> task);

static mutex browser_list_mutex;
static BrowserSource *first_browser = nullptr;

static void SendBrowserVisibility(CefRefPtr<CefBrowser> browser, bool isVisible)
{
	if (!browser)
		return;

#if ENABLE_WASHIDDEN
	if (isVisible) {
		browser->GetHost()->WasHidden(false);
		browser->GetHost()->Invalidate(PET_VIEW);
	} else {
		browser->GetHost()->WasHidden(true);
	}
#endif

	CefRefPtr<CefProcessMessage> msg =
		CefProcessMessage::Create("Visibility");
	CefRefPtr<CefListValue> args = msg->GetArgumentList();
	args->SetBool(0, isVisible);
	SendBrowserProcessMessage(browser, PID_RENDERER, msg);
}

void DispatchJSEvent(std::string eventName, std::string jsonString,
		     BrowserSource *browser = nullptr);

BrowserSource::BrowserSource(obs_data_t *, obs_source_t *source_)
	: source(source_)
{
	/* defer update */
	obs_source_update(source, nullptr);

	lock_guard<mutex> lock(browser_list_mutex);
	p_prev_next = &first_browser;
	next = first_browser;
	if (first_browser)
		first_browser->p_prev_next = &next;
	first_browser = this;
}

BrowserSource::~BrowserSource()
{
	DestroyBrowser();
	DestroyTextures();

	lock_guard<mutex> lock(browser_list_mutex);
	if (next)
		next->p_prev_next = p_prev_next;
	*p_prev_next = next;
}

void BrowserSource::ExecuteOnBrowser(BrowserFunc func, bool async)
{
	if (!async) {
#ifdef USE_QT_LOOP
		if (QThread::currentThread() == qApp->thread()) {
			if (!!cefBrowser)
				func(cefBrowser);
			return;
		}
#endif
		os_event_t *finishedEvent;
		os_event_init(&finishedEvent, OS_EVENT_TYPE_AUTO);
		bool success = QueueCEFTask([&]() {
			if (!!cefBrowser)
				func(cefBrowser);
			os_event_signal(finishedEvent);
		});
		if (success) {
			os_event_wait(finishedEvent);
		}
		os_event_destroy(finishedEvent);
	} else {
		CefRefPtr<CefBrowser> browser = cefBrowser;
		if (!!browser) {
#ifdef USE_QT_LOOP
			QueueBrowserTask(cefBrowser, func);
#else
			QueueCEFTask([=]() { func(browser); });
#endif
		}
	}
}

bool BrowserSource::CreateBrowser()
{
	return QueueCEFTask([this]() {
#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
		if (hwaccel) {
			obs_enter_graphics();
			tex_sharing_avail = gs_shared_texture_available();
			obs_leave_graphics();
		}
#else
		bool hwaccel = false;
#endif

		CefRefPtr<BrowserClient> browserClient = new BrowserClient(
			this, hwaccel && tex_sharing_avail, reroute_audio);

		CefWindowInfo windowInfo;
#if CHROME_VERSION_BUILD < 3071
		windowInfo.transparent_painting_enabled = true;
#endif
		windowInfo.width = width;
		windowInfo.height = height;
		windowInfo.windowless_rendering_enabled = true;

#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
		windowInfo.shared_texture_enabled = hwaccel;
#endif

		CefBrowserSettings cefBrowserSettings;

#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
		if (!fps_custom) {
			windowInfo.external_begin_frame_enabled = true;
			cefBrowserSettings.windowless_frame_rate = 0;
		} else {
			cefBrowserSettings.windowless_frame_rate = fps;
		}
#else
		cefBrowserSettings.windowless_frame_rate = fps;
#endif

#if ENABLE_LOCAL_FILE_URL_SCHEME
		if (is_local) {
			/* Disable web security for file:// URLs to allow
			 * local content access to remote APIs */
			cefBrowserSettings.web_security = STATE_DISABLED;
		}
#endif

		cefBrowser = CefBrowserHost::CreateBrowserSync(
			windowInfo, browserClient, url, cefBrowserSettings,
#if CHROME_VERSION_BUILD >= 3770
			CefRefPtr<CefDictionaryValue>(),
#endif
			nullptr);
#if CHROME_VERSION_BUILD >= 3683
		if (reroute_audio)
			cefBrowser->GetHost()->SetAudioMuted(true);
#endif

		SendBrowserVisibility(cefBrowser, is_showing);
	});
}

void BrowserSource::DestroyBrowser(bool async)
{
	ExecuteOnBrowser(
		[](CefRefPtr<CefBrowser> cefBrowser) {
			CefRefPtr<CefClient> client =
				cefBrowser->GetHost()->GetClient();
			BrowserClient *bc =
				reinterpret_cast<BrowserClient *>(client.get());
			if (bc) {
				bc->bs = nullptr;
			}

			/*
		 * This stops rendering
		 * http://magpcss.org/ceforum/viewtopic.php?f=6&t=12079
		 * https://bitbucket.org/chromiumembedded/cef/issues/1363/washidden-api-got-broken-on-branch-2062)
		 */
			cefBrowser->GetHost()->WasHidden(true);
			cefBrowser->GetHost()->CloseBrowser(true);
		},
		async);

	cefBrowser = nullptr;
}

void BrowserSource::ClearAudioStreams()
{
	QueueCEFTask([this]() {
		audio_streams.clear();
		std::lock_guard<std::mutex> lock(audio_sources_mutex);
		audio_sources.clear();
	});
}

void BrowserSource::SendMouseClick(const struct obs_mouse_event *event,
				   int32_t type, bool mouse_up,
				   uint32_t click_count)
{
	uint32_t modifiers = event->modifiers;
	int32_t x = event->x;
	int32_t y = event->y;

	ExecuteOnBrowser(
		[=](CefRefPtr<CefBrowser> cefBrowser) {
			CefMouseEvent e;
			e.modifiers = modifiers;
			e.x = x;
			e.y = y;
			CefBrowserHost::MouseButtonType buttonType =
				(CefBrowserHost::MouseButtonType)type;
			cefBrowser->GetHost()->SendMouseClickEvent(
				e, buttonType, mouse_up, click_count);
		},
		true);
}

void BrowserSource::SendMouseMove(const struct obs_mouse_event *event,
				  bool mouse_leave)
{
	uint32_t modifiers = event->modifiers;
	int32_t x = event->x;
	int32_t y = event->y;

	ExecuteOnBrowser(
		[=](CefRefPtr<CefBrowser> cefBrowser) {
			CefMouseEvent e;
			e.modifiers = modifiers;
			e.x = x;
			e.y = y;
			cefBrowser->GetHost()->SendMouseMoveEvent(e,
								  mouse_leave);
		},
		true);
}

void BrowserSource::SendMouseWheel(const struct obs_mouse_event *event,
				   int x_delta, int y_delta)
{
	uint32_t modifiers = event->modifiers;
	int32_t x = event->x;
	int32_t y = event->y;

	ExecuteOnBrowser(
		[=](CefRefPtr<CefBrowser> cefBrowser) {
			CefMouseEvent e;
			e.modifiers = modifiers;
			e.x = x;
			e.y = y;
			cefBrowser->GetHost()->SendMouseWheelEvent(e, x_delta,
								   y_delta);
		},
		true);
}

void BrowserSource::SendFocus(bool focus)
{
	ExecuteOnBrowser(
		[=](CefRefPtr<CefBrowser> cefBrowser) {
			cefBrowser->GetHost()->SendFocusEvent(focus);
		},
		true);
}

#ifdef __LINUX__

uint32_t KeyboardCodeFromXKeysym(unsigned int keysym)
{
	switch (keysym) {
	case XK_BackSpace:
		return VKEY_BACK;
	case XK_Delete:
	case XK_KP_Delete:
		return VKEY_DELETE;
	case XK_Tab:
	case XK_KP_Tab:
	case XK_ISO_Left_Tab:
	case XK_3270_BackTab:
		return VKEY_TAB;
	case XK_Linefeed:
	case XK_Return:
	case XK_KP_Enter:
	case XK_ISO_Enter:
		return VKEY_RETURN;
	case XK_Clear:
	case XK_KP_Begin: // NumPad 5 without Num Lock, for crosbug.com/29169.
		return VKEY_CLEAR;
	case XK_KP_Space:
	case XK_space:
		return VKEY_SPACE;
	case XK_Home:
	case XK_KP_Home:
		return VKEY_HOME;
	case XK_End:
	case XK_KP_End:
		return VKEY_END;
	case XK_Page_Up:
	case XK_KP_Page_Up: // aka XK_KP_Prior
		return VKEY_PRIOR;
	case XK_Page_Down:
	case XK_KP_Page_Down: // aka XK_KP_Next
		return VKEY_NEXT;
	case XK_Left:
	case XK_KP_Left:
		return VKEY_LEFT;
	case XK_Right:
	case XK_KP_Right:
		return VKEY_RIGHT;
	case XK_Down:
	case XK_KP_Down:
		return VKEY_DOWN;
	case XK_Up:
	case XK_KP_Up:
		return VKEY_UP;
	case XK_Escape:
		return VKEY_ESCAPE;
	case XK_Kana_Lock:
	case XK_Kana_Shift:
		return VKEY_KANA;
	case XK_Hangul:
		return VKEY_HANGUL;
	case XK_Hangul_Hanja:
		return VKEY_HANJA;
	case XK_Kanji:
		return VKEY_KANJI;
	case XK_Henkan:
		return VKEY_CONVERT;
	case XK_Muhenkan:
		return VKEY_NONCONVERT;
	case XK_A:
	case XK_a:
		return VKEY_A;
	case XK_B:
	case XK_b:
		return VKEY_B;
	case XK_C:
	case XK_c:
		return VKEY_C;
	case XK_D:
	case XK_d:
		return VKEY_D;
	case XK_E:
	case XK_e:
		return VKEY_E;
	case XK_F:
	case XK_f:
		return VKEY_F;
	case XK_G:
	case XK_g:
		return VKEY_G;
	case XK_H:
	case XK_h:
		return VKEY_H;
	case XK_I:
	case XK_i:
		return VKEY_I;
	case XK_J:
	case XK_j:
		return VKEY_J;
	case XK_K:
	case XK_k:
		return VKEY_K;
	case XK_L:
	case XK_l:
		return VKEY_L;
	case XK_M:
	case XK_m:
		return VKEY_M;
	case XK_N:
	case XK_n:
		return VKEY_N;
	case XK_O:
	case XK_o:
		return VKEY_O;
	case XK_P:
	case XK_p:
		return VKEY_P;
	case XK_Q:
	case XK_q:
		return VKEY_Q;
	case XK_R:
	case XK_r:
		return VKEY_R;
	case XK_S:
	case XK_s:
		return VKEY_S;
	case XK_T:
	case XK_t:
		return VKEY_T;
	case XK_U:
	case XK_u:
		return VKEY_U;
	case XK_V:
	case XK_v:
		return VKEY_V;
	case XK_W:
	case XK_w:
		return VKEY_W;
	case XK_X:
	case XK_x:
		return VKEY_X;
	case XK_Y:
	case XK_y:
		return VKEY_Y;
	case XK_Z:
	case XK_z:
		return VKEY_Z;

	case XK_0:
	case XK_1:
	case XK_2:
	case XK_3:
	case XK_4:
	case XK_5:
	case XK_6:
	case XK_7:
	case XK_8:
	case XK_9:
		return static_cast<unsigned int>(VKEY_0 + (keysym - XK_0));

	case XK_parenright:
		return VKEY_0;
	case XK_exclam:
		return VKEY_1;
	case XK_at:
		return VKEY_2;
	case XK_numbersign:
		return VKEY_3;
	case XK_dollar:
		return VKEY_4;
	case XK_percent:
		return VKEY_5;
	case XK_asciicircum:
		return VKEY_6;
	case XK_ampersand:
		return VKEY_7;
	case XK_asterisk:
		return VKEY_8;
	case XK_parenleft:
		return VKEY_9;

	case XK_KP_0:
	case XK_KP_1:
	case XK_KP_2:
	case XK_KP_3:
	case XK_KP_4:
	case XK_KP_5:
	case XK_KP_6:
	case XK_KP_7:
	case XK_KP_8:
	case XK_KP_9:
		return static_cast<unsigned int>(VKEY_NUMPAD0 +
						 (keysym - XK_KP_0));

	case XK_multiply:
	case XK_KP_Multiply:
		return VKEY_MULTIPLY;
	case XK_KP_Add:
		return VKEY_ADD;
	case XK_KP_Separator:
		return VKEY_SEPARATOR;
	case XK_KP_Subtract:
		return VKEY_SUBTRACT;
	case XK_KP_Decimal:
		return VKEY_DECIMAL;
	case XK_KP_Divide:
		return VKEY_DIVIDE;
	case XK_KP_Equal:
	case XK_equal:
	case XK_plus:
		return VKEY_OEM_PLUS;
	case XK_comma:
	case XK_less:
		return VKEY_OEM_COMMA;
	case XK_minus:
	case XK_underscore:
		return VKEY_OEM_MINUS;
	case XK_greater:
	case XK_period:
		return VKEY_OEM_PERIOD;
	case XK_colon:
	case XK_semicolon:
		return VKEY_OEM_1;
	case XK_question:
	case XK_slash:
		return VKEY_OEM_2;
	case XK_asciitilde:
	case XK_quoteleft:
		return VKEY_OEM_3;
	case XK_bracketleft:
	case XK_braceleft:
		return VKEY_OEM_4;
	case XK_backslash:
	case XK_bar:
		return VKEY_OEM_5;
	case XK_bracketright:
	case XK_braceright:
		return VKEY_OEM_6;
	case XK_quoteright:
	case XK_quotedbl:
		return VKEY_OEM_7;
	case XK_ISO_Level5_Shift:
		return VKEY_OEM_8;
	case XK_Shift_L:
	case XK_Shift_R:
		return VKEY_SHIFT;
	case XK_Control_L:
	case XK_Control_R:
		return VKEY_CONTROL;
	case XK_Meta_L:
	case XK_Meta_R:
	case XK_Alt_L:
	case XK_Alt_R:
		return VKEY_MENU;
	case XK_ISO_Level3_Shift:
		return VKEY_ALTGR;
	case XK_Multi_key:
		return VKEY_COMPOSE;
	case XK_Pause:
		return VKEY_PAUSE;
	case XK_Caps_Lock:
		return VKEY_CAPITAL;
	case XK_Num_Lock:
		return VKEY_NUMLOCK;
	case XK_Scroll_Lock:
		return VKEY_SCROLL;
	case XK_Select:
		return VKEY_SELECT;
	case XK_Print:
		return VKEY_PRINT;
	case XK_Execute:
		return VKEY_EXECUTE;
	case XK_Insert:
	case XK_KP_Insert:
		return VKEY_INSERT;
	case XK_Help:
		return VKEY_HELP;
	case XK_Super_L:
		return VKEY_LWIN;
	case XK_Super_R:
		return VKEY_RWIN;
	case XK_Menu:
		return VKEY_APPS;
	case XK_F1:
	case XK_F2:
	case XK_F3:
	case XK_F4:
	case XK_F5:
	case XK_F6:
	case XK_F7:
	case XK_F8:
	case XK_F9:
	case XK_F10:
	case XK_F11:
	case XK_F12:
	case XK_F13:
	case XK_F14:
	case XK_F15:
	case XK_F16:
	case XK_F17:
	case XK_F18:
	case XK_F19:
	case XK_F20:
	case XK_F21:
	case XK_F22:
	case XK_F23:
	case XK_F24:
		return static_cast<unsigned int>(VKEY_F1 + (keysym - XK_F1));
	case XK_KP_F1:
	case XK_KP_F2:
	case XK_KP_F3:
	case XK_KP_F4:
		return static_cast<unsigned int>(VKEY_F1 + (keysym - XK_KP_F1));

	case XK_guillemotleft:
	case XK_guillemotright:
	case XK_degree:
	// In the case of canadian multilingual keyboard layout, VKEY_OEM_102 is
	// assigned to ugrave key.
	case XK_ugrave:
	case XK_Ugrave:
	case XK_brokenbar:
		return VKEY_OEM_102; // international backslash key in 102 keyboard.

	// When evdev is in use, /usr/share/X11/xkb/symbols/inet maps F13-18 keys
	// to the special XF86XK symbols to support Microsoft Ergonomic keyboards:
	// https://bugs.freedesktop.org/show_bug.cgi?id=5783
	// In Chrome, we map these X key symbols back to F13-18 since we don't have
	// VKEYs for these XF86XK symbols.
	case XF86XK_Tools:
		return VKEY_F13;
	case XF86XK_Launch5:
		return VKEY_F14;
	case XF86XK_Launch6:
		return VKEY_F15;
	case XF86XK_Launch7:
		return VKEY_F16;
	case XF86XK_Launch8:
		return VKEY_F17;
	case XF86XK_Launch9:
		return VKEY_F18;
	case XF86XK_Refresh:
	case XF86XK_History:
	case XF86XK_OpenURL:
	case XF86XK_AddFavorite:
	case XF86XK_Go:
	case XF86XK_ZoomIn:
	case XF86XK_ZoomOut:
		// ui::AcceleratorGtk tries to convert the XF86XK_ keysyms on Chrome
		// startup. It's safe to return VKEY_UNKNOWN here since ui::AcceleratorGtk
		// also checks a Gdk keysym. http://crbug.com/109843
		return VKEY_UNKNOWN;
	// For supporting multimedia buttons on a USB keyboard.
	case XF86XK_Back:
		return VKEY_BROWSER_BACK;
	case XF86XK_Forward:
		return VKEY_BROWSER_FORWARD;
	case XF86XK_Reload:
		return VKEY_BROWSER_REFRESH;
	case XF86XK_Stop:
		return VKEY_BROWSER_STOP;
	case XF86XK_Search:
		return VKEY_BROWSER_SEARCH;
	case XF86XK_Favorites:
		return VKEY_BROWSER_FAVORITES;
	case XF86XK_HomePage:
		return VKEY_BROWSER_HOME;
	case XF86XK_AudioMute:
		return VKEY_VOLUME_MUTE;
	case XF86XK_AudioLowerVolume:
		return VKEY_VOLUME_DOWN;
	case XF86XK_AudioRaiseVolume:
		return VKEY_VOLUME_UP;
	case XF86XK_AudioNext:
		return VKEY_MEDIA_NEXT_TRACK;
	case XF86XK_AudioPrev:
		return VKEY_MEDIA_PREV_TRACK;
	case XF86XK_AudioStop:
		return VKEY_MEDIA_STOP;
	case XF86XK_AudioPlay:
		return VKEY_MEDIA_PLAY_PAUSE;
	case XF86XK_Mail:
		return VKEY_MEDIA_LAUNCH_MAIL;
	case XF86XK_LaunchA: // F3 on an Apple keyboard.
		return VKEY_MEDIA_LAUNCH_APP1;
	case XF86XK_LaunchB: // F4 on an Apple keyboard.
	case XF86XK_Calculator:
		return VKEY_MEDIA_LAUNCH_APP2;
	case XF86XK_WLAN:
		return VKEY_WLAN;
	case XF86XK_PowerOff:
		return VKEY_POWER;
	case XF86XK_MonBrightnessDown:
		return VKEY_BRIGHTNESS_DOWN;
	case XF86XK_MonBrightnessUp:
		return VKEY_BRIGHTNESS_UP;
	case XF86XK_KbdBrightnessDown:
		return VKEY_KBD_BRIGHTNESS_DOWN;
	case XF86XK_KbdBrightnessUp:
		return VKEY_KBD_BRIGHTNESS_UP;

		// TODO(sad): some keycodes are still missing.
	}
	return VKEY_UNKNOWN;
}

#endif

void BrowserSource::SendKeyClick(const struct obs_key_event *event, bool key_up)
{
	uint32_t modifiers = event->modifiers;
	std::string text = event->text;
#ifdef __LINUX__
	uint32_t native_vkey = KeyboardCodeFromXKeysym(event->native_vkey);
#else
	uint32_t native_vkey = event->native_vkey;
#endif
	uint32_t native_scancode = event->native_scancode;
	uint32_t native_modifiers = event->native_modifiers;

	ExecuteOnBrowser(
		[=](CefRefPtr<CefBrowser> cefBrowser) {
			CefKeyEvent e;
			e.windows_key_code = native_vkey;
			e.native_key_code = native_scancode;

			e.type = key_up ? KEYEVENT_KEYUP : KEYEVENT_RAWKEYDOWN;

			if (!text.empty()) {
				wstring wide = to_wide(text);
				if (wide.size())
					e.character = wide[0];
			}

			//e.native_key_code = native_vkey;
			e.modifiers = native_modifiers;

			cefBrowser->GetHost()->SendKeyEvent(e);
			if (!text.empty() && !key_up) {
				e.type = KEYEVENT_CHAR;
#ifdef __LINUX__
				e.windows_key_code =
					KeyboardCodeFromXKeysym(e.character);
#else
				e.windows_key_code = e.character;
#endif
				e.native_key_code = native_scancode;
				cefBrowser->GetHost()->SendKeyEvent(e);
			}
		},
		true);
}

void BrowserSource::SetShowing(bool showing)
{
	is_showing = showing;

	if (shutdown_on_invisible) {
		if (showing) {
			Update();
		} else {
			DestroyBrowser(true);
		}
	} else {
		ExecuteOnBrowser(
			[=](CefRefPtr<CefBrowser> cefBrowser) {
				CefRefPtr<CefProcessMessage> msg =
					CefProcessMessage::Create("Visibility");
				CefRefPtr<CefListValue> args =
					msg->GetArgumentList();
				args->SetBool(0, showing);
				SendBrowserProcessMessage(cefBrowser,
							  PID_RENDERER, msg);
			},
			true);
		Json json = Json::object{{"visible", showing}};
		DispatchJSEvent("obsSourceVisibleChanged", json.dump(), this);
#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
		if (showing && !fps_custom) {
			reset_frame = false;
		}
#endif

		SendBrowserVisibility(cefBrowser, showing);
	}
}

void BrowserSource::SetActive(bool active)
{
	ExecuteOnBrowser(
		[=](CefRefPtr<CefBrowser> cefBrowser) {
			CefRefPtr<CefProcessMessage> msg =
				CefProcessMessage::Create("Active");
			CefRefPtr<CefListValue> args = msg->GetArgumentList();
			args->SetBool(0, active);
			SendBrowserProcessMessage(cefBrowser, PID_RENDERER,
						  msg);
		},
		true);
	Json json = Json::object{{"active", active}};
	DispatchJSEvent("obsSourceActiveChanged", json.dump(), this);
}

void BrowserSource::Refresh()
{
	ExecuteOnBrowser(
		[](CefRefPtr<CefBrowser> cefBrowser) {
			cefBrowser->ReloadIgnoreCache();
		},
		true);
}

#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
inline void BrowserSource::SignalBeginFrame()
{
	if (reset_frame) {
		ExecuteOnBrowser(
			[](CefRefPtr<CefBrowser> cefBrowser) {
				cefBrowser->GetHost()->SendExternalBeginFrame();
			},
			true);

		reset_frame = false;
	}
}
#endif

void BrowserSource::Update(obs_data_t *settings)
{
	if (settings) {
		bool n_is_local;
		int n_width;
		int n_height;
		bool n_fps_custom;
		int n_fps;
		bool n_shutdown;
		bool n_restart;
		bool n_reroute;
		std::string n_url;
		std::string n_css;

		n_is_local = obs_data_get_bool(settings, "is_local_file");
		n_width = (int)obs_data_get_int(settings, "width");
		n_height = (int)obs_data_get_int(settings, "height");
		n_fps_custom = obs_data_get_bool(settings, "fps_custom");
		n_fps = (int)obs_data_get_int(settings, "fps");
		n_shutdown = obs_data_get_bool(settings, "shutdown");
		n_restart = obs_data_get_bool(settings, "restart_when_active");
		n_css = obs_data_get_string(settings, "css");
		n_url = obs_data_get_string(settings,
					    n_is_local ? "local_file" : "url");
		n_reroute = obs_data_get_bool(settings, "reroute_audio");

		if (n_is_local) {
			n_url = CefURIEncode(n_url, false);

#ifdef _WIN32
			n_url.replace(n_url.find("%3A"), 3, ":");
#endif

			while (n_url.find("%5C") != std::string::npos)
				n_url.replace(n_url.find("%5C"), 3, "/");

			while (n_url.find("%2F") != std::string::npos)
				n_url.replace(n_url.find("%2F"), 3, "/");

#if !ENABLE_LOCAL_FILE_URL_SCHEME
			/* http://absolute/ based mapping for older CEF */
			n_url = "http://absolute/" + n_url;
#elif defined(_WIN32)
			/* Widows-style local file URL:
			 * file:///C:/file/path.webm */
			n_url = "file:///" + n_url;
#else
			/* UNIX-style local file URL:
			 * file:///home/user/file.webm */
			n_url = "file://" + n_url;
#endif
		}

#if ENABLE_LOCAL_FILE_URL_SCHEME
		if (astrcmpi_n(n_url.c_str(), "http://absolute/", 16) == 0) {
			/* Replace http://absolute/ URLs with file://
			 * URLs if file:// URLs are enabled */
			n_url = "file:///" + n_url.substr(16);
			n_is_local = true;
		}
#endif

		if (n_is_local == is_local && n_width == width &&
		    n_height == height && n_fps_custom == fps_custom &&
		    n_fps == fps && n_shutdown == shutdown_on_invisible &&
		    n_restart == restart && n_css == css && n_url == url &&
		    n_reroute == reroute_audio) {
			return;
		}

		is_local = n_is_local;
		width = n_width;
		height = n_height;
		fps = n_fps;
		fps_custom = n_fps_custom;
		shutdown_on_invisible = n_shutdown;
		reroute_audio = n_reroute;
		restart = n_restart;
		css = n_css;
		url = n_url;

		obs_source_set_audio_active(source, reroute_audio);
	}

	DestroyBrowser(true);
	DestroyTextures();
	ClearAudioStreams();
	if (!shutdown_on_invisible || obs_source_showing(source))
		create_browser = true;

	first_update = false;
}

void BrowserSource::Tick()
{
	if (create_browser && CreateBrowser())
		create_browser = false;
#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
	if (!fps_custom)
		reset_frame = true;
#endif
}

extern void ProcessCef();

void BrowserSource::Render()
{
	bool flip = false;
#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
	flip = hwaccel;
#endif

	if (texture) {
		gs_effect_t *effect =
			obs_get_base_effect(OBS_EFFECT_PREMULTIPLIED_ALPHA);
		while (gs_effect_loop(effect, "Draw"))
			obs_source_draw(texture, 0, 0, 0, 0, flip);
	}

#if EXPERIMENTAL_SHARED_TEXTURE_SUPPORT_ENABLED
	SignalBeginFrame();
#elif USE_QT_LOOP
	ProcessCef();
#endif
}

static void ExecuteOnBrowser(BrowserFunc func, BrowserSource *bs)
{
	lock_guard<mutex> lock(browser_list_mutex);

	if (bs) {
		BrowserSource *bsw = reinterpret_cast<BrowserSource *>(bs);
		bsw->ExecuteOnBrowser(func, true);
	}
}

static void ExecuteOnAllBrowsers(BrowserFunc func)
{
	lock_guard<mutex> lock(browser_list_mutex);

	BrowserSource *bs = first_browser;
	while (bs) {
		BrowserSource *bsw = reinterpret_cast<BrowserSource *>(bs);
		bsw->ExecuteOnBrowser(func, true);
		bs = bs->next;
	}
}

void DispatchJSEvent(std::string eventName, std::string jsonString,
		     BrowserSource *browser)
{
	const auto jsEvent = [=](CefRefPtr<CefBrowser> cefBrowser) {
		CefRefPtr<CefProcessMessage> msg =
			CefProcessMessage::Create("DispatchJSEvent");
		CefRefPtr<CefListValue> args = msg->GetArgumentList();

		args->SetString(0, eventName);
		args->SetString(1, jsonString);
		SendBrowserProcessMessage(cefBrowser, PID_RENDERER, msg);
	};

	if (!browser)
		ExecuteOnAllBrowsers(jsEvent);
	else
		ExecuteOnBrowser(jsEvent, browser);
}

