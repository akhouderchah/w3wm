#include "w3_DLL.h"
#include <iostream>
#include <tchar.h>
#include <windows.h>
#include "Shlwapi.h"

#include <unordered_map>

using namespace std;

#define T_WNDCLASS_NAME _T("w3wm_class")
#define T_APP_NAME _T("w3wm")

#define ARR_SIZE(arr) sizeof(arr)/sizeof(arr[0])

void UpdateHotkeys();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = T_WNDCLASS_NAME;

	if(!RegisterClassEx(&wc))
	{
		MessageBoxEx(NULL, _T("w3wm failed to register the window class"), _T("Error"), MB_OK | MB_ICONERROR, 0);
		return -1;
	}

	HWND hwnd = CreateWindowEx(0, T_WNDCLASS_NAME, T_APP_NAME, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
	if(!hwnd)
	{
		MessageBoxEx(NULL, _T("w3wm failed to create the window"), _T("Error"), MB_OK | MB_ICONERROR, 0);
		return -1;
	}

	InstallHooks(hwnd);
	UpdateHotkeys();

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	RemoveHooks();

	return (int)msg.wParam;
}

void UpdateHotkeys()
{
	// Initialize hotkeys with the defaults
	HotkeyDef defs[] = { HOTKEYS(F_HOTKEY_ARR) };

	// Modify defaults with the ini
	LPCTSTR names[] = { HOTKEYS(F_HOTKEY_NAME_ARR) };
	int max = ARR_SIZE(names);

	TCHAR iniDir[512];
	GetCurrentDirectory(512, iniDir);
	_tcscat_s(iniDir, 100, _T("\\config.ini"));

	if(!PathFileExists(iniDir))
	{
		max = 0;
		MessageBoxEx(NULL, iniDir, _T("Ini not found"), MB_OK, 0);
	}

	for(int i = 0; i < max; ++i)
	{
		TCHAR inBuf[80];
		DWORD res = GetPrivateProfileString(_T("Keybindings"), names[i], _T(""), inBuf, 80, iniDir);

		if(res != 0)
		{
			ParseHotkey(&defs[i], inBuf);
		}
	}

	// Inform DLL of hotkey mappings
	SetHotkeys(defs, ARR_SIZE(defs));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_DESTROY:
			PostQuitMessage(WM_QUIT);
			break;
		case WM_HOTKEY:
			switch(lParam)
			{
				case EH_WND_CLOSE:
					WinExec("C:\\Program Files\\CMake\\bin\\cmake-gui.exe", SW_RESTORE);
					break;
				case EH_WND_MOVE_DOWN:
					WinExec("C:\\Windows\\System32\\cmd.exe", SW_RESTORE);
					break;
				case EH_W3WM_CLOSE:
					PostQuitMessage(WM_QUIT);
					break;
				case EH_W3WM_RESTART:
					// TODO - actually restart everything
					UpdateHotkeys();
					break;
				default:
					break;
			}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}