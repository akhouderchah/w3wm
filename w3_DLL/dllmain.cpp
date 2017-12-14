// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <tchar.h>
#include "w3_DLL.h"

#pragma comment(linker, "/SECTION:.SHARED,RWS")
#pragma data_seg(".SHARED")
HHOOK hKeyboardHook = 0;
HWND hAppWnd = 0;

bool bHotkeysSet = false;

// Map of key -> HotkeyDef
// Note that memory in the shared segment must be initialized
HotkeyDef gs_Hotkeys[256] = { 0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0 };
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

LRESULT CALLBACK LLKeyboardProc( int code, WPARAM wParam, LPARAM lParam )
{
	if( code != 0 )
	{
		return CallNextHookEx( hKeyboardHook, code, wParam, lParam );
	}

	tagKBDLLHOOKSTRUCT *pKbStruct = (tagKBDLLHOOKSTRUCT*)lParam;
	HotkeyDef &hotkey = gs_Hotkeys[pKbStruct->vkCode];
	if(hotkey.m_MainKey != 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
	{
		if( ((hotkey.m_Modifiers & EM_WIN) && !(GetAsyncKeyState(VK_LWIN) & 0x8000) &&
				!(GetAsyncKeyState(VK_RWIN) & 0x8000)) ||
			((hotkey.m_Modifiers & EM_CTRL) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000)) ||
			((hotkey.m_Modifiers & EM_SHIFT) && !(GetAsyncKeyState(VK_SHIFT) & 0x8000)) ||
			((hotkey.m_Modifiers & EM_ALT) && !(GetAsyncKeyState(VK_MENU) & 0x8000)) )
		{
			goto exit;
		}

		SendMessage(hAppWnd, WM_HOTKEY, 99, hotkey.m_HotkeyID);
		return 1;
	}

exit:
	return CallNextHookEx( hKeyboardHook, code, wParam, lParam );
}

void W3_DLL_API InstallHooks(HWND appWnd)
{
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInstance, 0);
	hAppWnd = appWnd;
}

void W3_DLL_API RemoveHooks(void)
{
	UnhookWindowsHookEx(hKeyboardHook);
	hKeyboardHook = 0;
	hAppWnd = 0;
}

void W3_DLL_API SetHotkeys(HotkeyDef *pHotkeyDefs, size_t hotkeyCount)
{
	if(bHotkeysSet)
	{
		for(int i = 0; i < 256; ++i)
		{
			gs_Hotkeys[i] = {0, 0, 0};
		}
	}

	// @TODO - add support for multiple hotkeys w/same main key
	for(size_t i = 0; i < hotkeyCount; ++i)
	{
		gs_Hotkeys[pHotkeyDefs[i].m_MainKey] = pHotkeyDefs[i];
	}

	bHotkeysSet = true;
}