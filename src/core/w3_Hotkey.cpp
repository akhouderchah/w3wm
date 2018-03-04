#include "w3_Hotkey.h"
#include <windows.h>

void ParseHotkey(HotkeyDef *pHotkey, LPCTSTR str)
{
	int len = _tcslen(str);
	UINT8 mods = 0;

	if(len % 2 == 0)
	{
		goto error;
	}

	for(int i = 0; i < len-1; i+=2)
	{
		switch(str[i])
		{
			case _T('M'):
				mods |= EM_ALT;
				break;
			case _T('C'):
				mods |= EM_CTRL;
				break;
			case _T('S'):
				mods |= EM_SHIFT;
				break;
			case _T('W'):
				mods |= EM_WIN;
				break;
			default:
				goto error;
		}

		if(str[i+1] != TCHAR('-'))
		{
			goto error;
		}
	}

	pHotkey->m_MainKey = toupper(str[len-1]);
	pHotkey->m_Modifiers = mods;

	return;

error:
	// @TODO - more informative error message
	MessageBoxEx(NULL, _T("Invalid hotkey combination"), _T("Configuration Error"), MB_OK, 0);
	return;
}
