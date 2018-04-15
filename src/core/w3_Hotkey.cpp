#include "w3_Hotkey.h"
#include "w3_Context.h"
#include <windows.h>

void w3Context::ParseHotkey(LPTSTR tstr, const VirtualKeyMap &keyMap, HotkeyDef *pHotkey) const
{
	size_t len = _tcslen(tstr);
	LPSTR str;
	UINT8 mods = 0;
	VirtualKeyMap::const_iterator iter;

#ifdef _UNICODE
	// Create ascii string from tstr
	LPSTR mbStr = new char[len+1];
	WideCharToMultiByte(20127, WC_NO_BEST_FIT_CHARS, tstr, len+1, mbStr, len+1, '.', NULL);
	str = mbStr;
#else
	str = tstr;
#endif

	// Get all mods and store the remaining string in key
	LPSTR key = str;
	for(size_t i = 0; i < len-1; ++i)
	{
		if(str[i+1] == '-')
		{
			switch(tolower(str[i]))
			{
				case 'm':
					mods |= EM_ALT;
					break;
				case 'c':
					mods |= EM_CTRL;
					break;
				case 's':
					mods |= EM_SHIFT;
					break;
				case 'w':
					mods |= EM_WIN;
					break;
				default:
					goto error;
			}
			key = &str[i+2];
			++i;
		}
		else
		{
			break;
		}
	}

	// Map upper-case key string to virtual key code
	LPSTR temp = key;
	while(*temp)
	{
		*temp = toupper(*temp);
		++temp;
	}

	iter = keyMap.find(key);
	if(iter == keyMap.end())
	{
		goto error;
	}

	pHotkey->m_MainKey = iter->second;
	pHotkey->m_Modifiers = mods;

shutdown:
#ifdef _UNICODE
	// Free ascii string
	delete [] str;
#endif
	return;

error:
	// @TODO - more informative error message
	RELEASE_MESSAGE("Configuration Error", "Invalid hotkey combination: %s", key);
	goto shutdown;
}

bool w3Context::GenerateDefaultIni(LPCTSTR filename) const
{
	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	const char *str =
		"; w3wm Configuration File\r\n"
		"\r\n"
		"[Keybindings]\r\n"
		"; Syntax:\r\n"
		";;   FUNCTION=[MODKEY-]*BASEKEY\r\n"
		";;\r\n"
		";; Note that the syntax is case-insensitive. Thus function names, modkeys,\r\n"
		";; and basekeys may have any case.\r\n"
		";\r\n"
		"; Functions:\r\n"
		";; +--------------------+-------------------------------+\r\n"
		";; |   Function         |          Description          |\r\n"
		";; +--------------------+-------------------------------+\r\n"
		";; |  Focus_Up          |         Move focus up         |\r\n"
		";; |  Focus_Down        |         Move focus down       |\r\n"
		";; |  Focus_Right       |         Move focus right      |\r\n"
		";; |  Focus_Left        |         Move focus left       |\r\n"
		";; +--------------------+-------------------------------+\r\n"
		";; |  Wnd_Up            |        Move window up         |\r\n"
		";; |  Wnd_Down          |        Move window down       |\r\n"
		";; |  Wnd_Right         |        Move window right      |\r\n"
		";; |  Wnd_Left          |        Move window left       |\r\n"
		";; +--------------------+-------------------------------+\r\n"
		";; |  Wnd_Fullscreen    |   Fullscreen current window   |\r\n"
		";; |  Wnd_Close         |      Close current window     |\r\n"
		";; |  Wnd_Open          |        Open new window        |\r\n"
		";; |  W3wm_Open_Console |        Open new console       |\r\n"
		";; |  W3wm_Lock         |           Lock screen         |\r\n"
		";; |  W3wm_Restart      |          Restart w3wm         |\r\n"
		";; |  W3wm_Close        |           Close w3wm          |\r\n"
		";; +--------------------+-------------------------------+\r\n"
		";\r\n"
		"; Modkeys:\r\n"
		";; +-----+------------------+\r\n"
		";; | Key |    Description   |\r\n"
		";; +-----+------------------+\r\n"
		";; |  S  |     Shift Key    |\r\n"
		";; |  C  |    Control Key   |\r\n"
		";; |  W  |    Windows Key   |\r\n"
		";; |  M  |      Alt Key     |\r\n"
		";; +-----+------------------+\r\n"
		";\r\n"
		"; Basekeys:\r\n"
		";; For letters, numbers, and F1-F12 the basekey name is simply the intended key.\r\n"
		";; Special basekeys are presented below:\r\n"
		";; +--------------+------------------+\r\n"
		";; | Basekey Name |    Description   |\r\n"
		";; +--------------+------------------+\r\n"
		";; | Backspace    |  Backspace key   |\r\n"
		";; | Tab          |      Tab key     |\r\n"
		";; | Enter        |     Enter key    |\r\n"
		";; | Menu         |      Menu key    |\r\n"
		";; | Pause        |     Pause key    |\r\n"
		";; | Caps         |      Caps key    |\r\n"
		";; | Escape       |     Escape key   |\r\n"
		";; | Space        |     Space key    |\r\n"
		";; | PageUp       |    Page Up key   |\r\n"
		";; | PageDown     |   Page Down key  |\r\n"
		";; | End          |      End key     |\r\n"
		";; | Home         |      Home key    |\r\n"
		";; | Left         |  Left arrow key  |\r\n"
		";; | Up           |   Up arrow key   |\r\n"
		";; | Right        |  Right arrow key |\r\n"
		";; | Down         |  Down arrow key  |\r\n"
		";; | Print        |      Print key   |\r\n"
		";; | PrintScreen  | Print screen key |\r\n"
		";; | Insert       |     Insert key   |\r\n"
		";; | Delete       |     Delete key   |\r\n"
		";; +--------------+------------------+\r\n"
		";\r\n"
		";\r\n"
		"; Examples:\r\n"
		";\r\n"
		"; Close window with Win+Alt+Delete\r\n"
		";;; wnd_close=m-W-delete\r\n"
		";\r\n"
		"; Quit w3wm with Win+Shift+Escape\r\n"
		";;; w3wm_close=W-s-Escape\r\n"
		";\r\n"
		"; Move focus left with Ctrl+F\r\n"
		";;; focus_left=C-f\r\n"
		"\r\n"
		"[Applications]\r\n"
		"; Cmd=\"C:\\Program Files\\ConEmu\\ConEmu64.exe\"\r\n";

	BOOL success = true;
	DWORD bytesWritten;
	success = WriteFile(hFile, str, strlen(str), &bytesWritten, NULL);

	CloseHandle(hFile);
	return !!success;
}
