#pragma once

#include "stdafx.h"
#include "w3_Hotkey.h"

#ifdef W3_DLL_EXPORTS
#define W3_DLL_API __declspec(dllexport)
#else
#define W3_DLL_API __declspec(dllimport)
#endif

void W3_DLL_API InstallHooks(HWND appWnd);
void W3_DLL_API RemoveHooks(void);

void W3_DLL_API SetHotkeys(HotkeyDef *pHotkeyDefs, size_t hotkeyCount);