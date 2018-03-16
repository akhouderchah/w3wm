// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <tchar.h>
#include "w3_DLL.h"

#pragma comment(linker, "/SECTION:.SHARED,RWS")
#pragma data_seg(".SHARED")
HHOOK hKeyboardHook = 0;
HHOOK hCBTHook = 0;
HWND hAppWnd = 0;

bool bHotkeysSet = false;

// Each element is a bitfield indicating if a certain combination
// of modifiers, plus the given main key, has a hotkey mapped to it
UINT16 gs_ActiveKeys[256] = {};

// Unordered list of hotkeys
HotkeyDef gs_Hotkeys[256] = {};
#pragma data_seg()

HMODULE hInstance = 0;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInstance = (HINSTANCE)hModule;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

LRESULT CALLBACK LLKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if(code != 0)
	{
		return CallNextHookEx( hKeyboardHook, code, wParam, lParam );
	}

	tagKBDLLHOOKSTRUCT *pKbStruct = (tagKBDLLHOOKSTRUCT*)lParam;
	UINT8 keyDown = pKbStruct->vkCode;
	UINT16 keyFlag = gs_ActiveKeys[keyDown];
	if(keyFlag != 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
	{
		UINT16 mods = 0;
		mods |= EM_CTRL * !!(GetAsyncKeyState(VK_CONTROL) & 0x8000);
		mods |= EM_ALT * !!(GetAsyncKeyState(VK_MENU) & 0x8000);
		mods |= EM_WIN * (!!(GetAsyncKeyState(VK_LWIN) & 0x8000) || !!(GetAsyncKeyState(VK_RWIN) & 0x8000));
		mods |= EM_SHIFT * !!(GetAsyncKeyState(VK_SHIFT) & 0x8000);

		if(keyFlag & (1 << mods))
		{
			HotkeyDef *pHotkey = gs_Hotkeys;
			while(1)
			{
				if(pHotkey->m_MainKey == keyDown && pHotkey->m_Modifiers == mods)
				{
					SendMessage(hAppWnd, WM_HOTKEY, 99, pHotkey->m_HotkeyID);
					/*
					INPUT in;
					in.type = INPUT_KEYBOARD;
					in.ki.wScan = 0;
					in.ki.time = 0;
					in.ki.dwExtraInfo = 0;
					in.ki.wVk = VK_LWIN;
					in.ki.dwFlags = KEYEVENTF_KEYUP;
					SendInput(1, &in, sizeof(INPUT));
					*/
					return 1;
				}

				++pHotkey;
			}
		}
	}

	return CallNextHookEx( hKeyboardHook, code, wParam, lParam );
}

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
	{
		return CallNextHookEx(hCBTHook, nCode, wParam, lParam);
	}

	switch(nCode)
	{
	case HCBT_MOVESIZE:
	case HCBT_SYSCOMMAND:
		return 1;
	default:
		return CallNextHookEx(hCBTHook, nCode, wParam, lParam);
	}
}

void W3_DLL_API InstallHooks(HWND appWnd)
{
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInstance, 0);
	hCBTHook = SetWindowsHookEx(WH_CBT, CBTProc, hInstance, 0);
	hAppWnd = appWnd;
}

void W3_DLL_API RemoveHooks(void)
{
	UnhookWindowsHookEx(hKeyboardHook);
	UnhookWindowsHookEx(hCBTHook);
	hKeyboardHook = 0;
	hCBTHook = 0;
	hAppWnd = 0;
}

void W3_DLL_API SetHotkeys(HotkeyDef *pHotkeyDefs, size_t hotkeyCount)
{
	if(bHotkeysSet)
	{
		for(int i = 0; i < 256; ++i)
		{
			gs_ActiveKeys[i] = 0;
			gs_Hotkeys[i] = {0, 0, 0};
		}
	}

	for(size_t i = 0; i < hotkeyCount; ++i)
	{
		gs_ActiveKeys[pHotkeyDefs[i].m_MainKey] |= (1 << pHotkeyDefs[i].m_Modifiers);
		gs_Hotkeys[i] = pHotkeyDefs[i];
	}

	bHotkeysSet = true;
}
