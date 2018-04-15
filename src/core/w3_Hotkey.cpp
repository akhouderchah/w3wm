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
