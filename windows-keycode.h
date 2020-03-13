#ifndef VKEY_UNKNOWN

#define VKEY_UNKNOWN 0

// POSIX specific VKEYs. Note that as of Windows SDK 7.1, 0x97-9F, 0xD8-DA,
// and 0xE8 are unassigned.
#define VKEY_WLAN 0x97
#define VKEY_POWER 0x98
#define VKEY_BRIGHTNESS_DOWN 0xD8
#define VKEY_BRIGHTNESS_UP 0xD9
#define VKEY_KBD_BRIGHTNESS_DOWN 0xDA
#define VKEY_KBD_BRIGHTNESS_UP 0xE8

// Windows does not have a specific key code for AltGr. We use the unused 0xE1
// (VK_OEM_AX) code to represent AltGr, matching the behaviour of Firefox on
// Linux.
#define VKEY_ALTGR 0xE1
// Windows does not have a specific key code for Compose. We use the unused
// 0xE6 (VK_ICO_CLEAR) code to represent Compose.
#define VKEY_COMPOSE 0xE6

// Left mouse button
#ifndef VKEY_LBUTTON
#define VKEY_LBUTTON 0x01
#endif
// Right mouse button
#ifndef VKEY_RBUTTON
#define VKEY_RBUTTON 0x02
#endif
// Middle mouse button (three-button mouse)
#ifndef VKEY_MBUTTON
#define VKEY_MBUTTON 0x04
#endif
#ifndef VKEY_XBUTTON1
#define VKEY_XBUTTON1 0x05
#endif
#ifndef VKEY_XBUTTON2
#define VKEY_XBUTTON2 0x06
#endif

#ifndef VKEY_BACK
#define VKEY_BACK 0x08
#endif
#ifndef VKEY_TAB
#define VKEY_TAB 0x09
#endif
#ifndef VKEY_CLEAR
#define VKEY_CLEAR 0x0C
#endif
#ifndef VKEY_RETURN
#define VKEY_RETURN 0x0D
#endif
#ifndef VKEY_SHIFT
#define VKEY_SHIFT 0x10
#endif
#ifndef VKEY_CONTROL
#define VKEY_CONTROL 0x11 // CTRL key
#endif
#ifndef VKEY_MENU
#define VKEY_MENU 0x12 // ALT key
#endif
#ifndef VKEY_PAUSE
#define VKEY_PAUSE 0x13 // PAUSE key
#endif
#ifndef VKEY_CAPITAL
#define VKEY_CAPITAL 0x14 // CAPS LOCK key
#endif
#ifndef VKEY_KANA
#define VKEY_KANA 0x15 // Input Method Editor (IME) Kana mode
#endif
#ifndef VKEY_HANGUL
#define VKEY_HANGUL 0x15 // IME Hangul mode
#endif
#ifndef VKEY_JUNJA
#define VKEY_JUNJA 0x17 // IME Junja mode
#endif
#ifndef VKEY_FINAL
#define VKEY_FINAL 0x18 // IME final mode
#endif
#ifndef VKEY_HANJA
#define VKEY_HANJA 0x19 // IME Hanja mode
#endif
#ifndef VKEY_KANJI
#define VKEY_KANJI 0x19 // IME Kanji mode
#endif
#ifndef VKEY_ESCAPE
#define VKEY_ESCAPE 0x1B // ESC key
#endif
#ifndef VKEY_CONVERT
#define VKEY_CONVERT 0x1C // IME convert
#endif
#ifndef VKEY_NONCONVERT
#define VKEY_NONCONVERT 0x1D // IME nonconvert
#endif
#ifndef VKEY_ACCEPT
#define VKEY_ACCEPT 0x1E // IME accept
#endif
#ifndef VKEY_MODECHANGE
#define VKEY_MODECHANGE 0x1F // IME mode change request
#endif
#ifndef VKEY_SPACE
#define VKEY_SPACE 0x20 // SPACE key
#endif
#ifndef VKEY_PRIOR
#define VKEY_PRIOR 0x21 // PAGE UP key
#endif
#ifndef VKEY_NEXT
#define VKEY_NEXT 0x22 // PAGE DOWN key
#endif
#ifndef VKEY_END
#define VKEY_END 0x23 // END key
#endif
#ifndef VKEY_HOME
#define VKEY_HOME 0x24 // HOME key
#endif
#ifndef VKEY_LEFT
#define VKEY_LEFT 0x25 // LEFT ARROW key
#endif
#ifndef VKEY_UP
#define VKEY_UP 0x26 // UP ARROW key
#endif
#ifndef VKEY_RIGHT
#define VKEY_RIGHT 0x27 // RIGHT ARROW key
#endif
#ifndef VKEY_DOWN
#define VKEY_DOWN 0x28 // DOWN ARROW key
#endif
#ifndef VKEY_SELECT
#define VKEY_SELECT 0x29 // SELECT key
#endif
#ifndef VKEY_PRINT
#define VKEY_PRINT 0x2A // PRINT key
#endif
#ifndef VKEY_EXECUTE
#define VKEY_EXECUTE 0x2B // EXECUTE key
#endif
#ifndef VKEY_SNAPSHOT
#define VKEY_SNAPSHOT 0x2C // PRINT SCREEN key
#endif
#ifndef VKEY_INSERT
#define VKEY_INSERT 0x2D // INS key
#endif
#ifndef VKEY_DELETE
#define VKEY_DELETE 0x2E // DEL key
#endif
#ifndef VKEY_HELP
#define VKEY_HELP 0x2F // HELP key
#endif

#define VKEY_0 0x30
#define VKEY_1 0x31
#define VKEY_2 0x32
#define VKEY_3 0x33
#define VKEY_4 0x34
#define VKEY_5 0x35
#define VKEY_6 0x36
#define VKEY_7 0x37
#define VKEY_8 0x38
#define VKEY_9 0x39
#define VKEY_A 0x41
#define VKEY_B 0x42
#define VKEY_C 0x43
#define VKEY_D 0x44
#define VKEY_E 0x45
#define VKEY_F 0x46
#define VKEY_G 0x47
#define VKEY_H 0x48
#define VKEY_I 0x49
#define VKEY_J 0x4A
#define VKEY_K 0x4B
#define VKEY_L 0x4C
#define VKEY_M 0x4D
#define VKEY_N 0x4E
#define VKEY_O 0x4F
#define VKEY_P 0x50
#define VKEY_Q 0x51
#define VKEY_R 0x52
#define VKEY_S 0x53
#define VKEY_T 0x54
#define VKEY_U 0x55
#define VKEY_V 0x56
#define VKEY_W 0x57
#define VKEY_X 0x58
#define VKEY_Y 0x59
#define VKEY_Z 0x5A

#define VKEY_LWIN 0x5B // Left Windows key (Microsoft Natural keyboard)

#define VKEY_RWIN 0x5C // Right Windows key (Natural keyboard)

#define VKEY_APPS 0x5D // Applications key (Natural keyboard)

#define VKEY_SLEEP 0x5F // Computer Sleep key

// Num pad keys
#define VKEY_NUMPAD0 0x60
#define VKEY_NUMPAD1 0x61
#define VKEY_NUMPAD2 0x62
#define VKEY_NUMPAD3 0x63
#define VKEY_NUMPAD4 0x64
#define VKEY_NUMPAD5 0x65
#define VKEY_NUMPAD6 0x66
#define VKEY_NUMPAD7 0x67
#define VKEY_NUMPAD8 0x68
#define VKEY_NUMPAD9 0x69
#define VKEY_MULTIPLY 0x6A
#define VKEY_ADD 0x6B
#define VKEY_SEPARATOR 0x6C
#define VKEY_SUBTRACT 0x6D
#define VKEY_DECIMAL 0x6E
#define VKEY_DIVIDE 0x6F

#define VKEY_F1 0x70
#define VKEY_F2 0x71
#define VKEY_F3 0x72
#define VKEY_F4 0x73
#define VKEY_F5 0x74
#define VKEY_F6 0x75
#define VKEY_F7 0x76
#define VKEY_F8 0x77
#define VKEY_F9 0x78
#define VKEY_F10 0x79
#define VKEY_F11 0x7A
#define VKEY_F12 0x7B
#define VKEY_F13 0x7C
#define VKEY_F14 0x7D
#define VKEY_F15 0x7E
#define VKEY_F16 0x7F
#define VKEY_F17 0x80
#define VKEY_F18 0x81
#define VKEY_F19 0x82
#define VKEY_F20 0x83
#define VKEY_F21 0x84
#define VKEY_F22 0x85
#define VKEY_F23 0x86
#define VKEY_F24 0x87

#define VKEY_NUMLOCK 0x90
#define VKEY_SCROLL 0x91
#define VKEY_LSHIFT 0xA0
#define VKEY_RSHIFT 0xA1
#define VKEY_LCONTROL 0xA2
#define VKEY_RCONTROL 0xA3
#define VKEY_LMENU 0xA4
#define VKEY_RMENU 0xA5

#define VKEY_BROWSER_BACK 0xA6      // Windows 2000/XP: Browser Back key
#define VKEY_BROWSER_FORWARD 0xA7   // Windows 2000/XP: Browser Forward key
#define VKEY_BROWSER_REFRESH 0xA8   // Windows 2000/XP: Browser Refresh key
#define VKEY_BROWSER_STOP 0xA9      // Windows 2000/XP: Browser Stop key
#define VKEY_BROWSER_SEARCH 0xAA    // Windows 2000/XP: Browser Search key
#define VKEY_BROWSER_FAVORITES 0xAB // Windows 2000/XP: Browser Favorites key
#define VKEY_BROWSER_HOME 0xAC // Windows 2000/XP: Browser Start and Home key
#define VKEY_VOLUME_MUTE 0xAD  // Windows 2000/XP: Volume Mute key
#define VKEY_VOLUME_DOWN 0xAE  // Windows 2000/XP: Volume Down key
#define VKEY_VOLUME_UP 0xAF    // Windows 2000/XP: Volume Up key
#define VKEY_MEDIA_NEXT_TRACK 0xB0  // Windows 2000/XP: Next Track key
#define VKEY_MEDIA_PREV_TRACK 0xB1  // Windows 2000/XP: Previous Track key
#define VKEY_MEDIA_STOP 0xB2        // Windows 2000/XP: Stop Media key
#define VKEY_MEDIA_PLAY_PAUSE 0xB3  // Windows 2000/XP: Play/Pause Media key
#define VKEY_MEDIA_LAUNCH_MAIL 0xB4 // Windows 2000/XP: Start Mail key
#define VKEY_MEDIA_LAUNCH_MEDIA_SELECT 0xB5 // Windows 2000/XP: Select Media key
#define VKEY_MEDIA_LAUNCH_APP1 \
	0xB6 // VKEY_LAUNCH_APP1 (B6) Windows 2000/XP: Start Application 1 key
#define VKEY_MEDIA_LAUNCH_APP2 \
	0xB7 // VKEY_LAUNCH_APP2 (B7) Windows 2000/XP: Start Application 2 key

// VKEY_OEM_1 (BA) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ';:' key
#define VKEY_OEM_1 0xBA

// Windows 2000/XP: For any country/region, the '+' key
#define VKEY_OEM_PLUS 0xBB

// Windows 2000/XP: For any country/region, the ',' key
#define VKEY_OEM_COMMA 0xBC

// Windows 2000/XP: For any country/region, the '-' key
#define VKEY_OEM_MINUS 0xBD

// Windows 2000/XP: For any country/region, the '.' key
#define VKEY_OEM_PERIOD 0xBE

// VKEY_OEM_2 (BF) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '/?' key
#define VKEY_OEM_2 0xBF

// VKEY_OEM_3 (C0) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '`~' key
#define VKEY_OEM_3 0xC0

// VKEY_OEM_4 (DB) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '[{' key
#define VKEY_OEM_4 0xDB

// VKEY_OEM_5 (DC) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '\|' key
#define VKEY_OEM_5 0xDC

// VKEY_OEM_6 (DD) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ']}' key
#define VKEY_OEM_6 0xDD

// VKEY_OEM_7 (DE) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the 'single-quote/double-quote' key
#define VKEY_OEM_7 0xDE

// VKEY_OEM_8 (DF) Used for miscellaneous characters; it can vary by keyboard.
#define VKEY_OEM_8 0xDF

// VKEY_OEM_102 (E2) Windows 2000/XP: Either the angle bracket key or the backslash key on the RT 102-key keyboard
#define VKEY_OEM_102 0xE2

#define VKEY_OEM_BACKTAB 0xF5
#define VKEY_OEM_FJ_TOUROKU 0x94
#define VKEY_OEM_FJ_MASSHOU 0x93

// Windows 95/98/Me, Windows NT 4.0, Windows 2000/XP: IME PROCESS key
#define VKEY_PROCESSKEY 0xE5

// Windows 2000/XP: Used to pass Unicode characters as if they were keystrokes. The VKEY_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT,SendInput, WM_KEYDOWN, and WM_KEYUP
#define VKEY_PACKET 0xE7

#define VKEY_ATTN 0xF6  // Attn key
#define VKEY_CRSEL 0xF7 // CrSel key
#define VKEY_EXSEL 0xF8 // ExSel key
#define VKEY_EREOF 0xF9 // Erase EOF key
#define VKEY_PLAY 0xFA  // Play key
#define VKEY_ZOOM 0xFB  // Zoom key

#define VKEY_NONAME 0xFC // Reserved for future use

#define VKEY_PA1 0xFD // VKEY_PA1 (FD) PA1 key

#define VKEY_OEM_CLEAR 0xFE // Clear key

#endif // VKEY_UNKNOWN
