#include "w3_Context.h"
#include "w3_Hotkey.h"

#include <tchar.h>
#include <windows.h>

w3Context g_Context;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	g_Context.Initialize(hInstance);

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	g_Context.Shutdown();

	return (int)msg.wParam;
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
					break;
				case EH_WND_MOVE_DOWN:
					break;
				case EH_W3WM_CLOSE:
					g_Context.Shutdown();
					PostQuitMessage(WM_QUIT);
					break;
				case EH_W3WM_RESTART:
					g_Context.Restart();
					break;
				case EH_W3WM_LOCK:
					g_Context.LockScreen();
					break;
				case EH_W3WM_OPEN_CONSOLE:
					g_Context.OpenConsole();
					break;
				default:
					break;
			}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
