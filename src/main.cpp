#include "w3_Context.h"
#include "w3_Hotkey.h"

#include <tchar.h>
#include <windows.h>

w3Context g_Context;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow)
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
				case EH_FOCUS_UP:
				case EH_FOCUS_RIGHT:
				case EH_FOCUS_DOWN:
				case EH_FOCUS_LEFT:
					g_Context.MoveFocus(EGridDirection(lParam-EH_FOCUS_UP));
					break;
				case EH_WND_UP:
				case EH_WND_RIGHT:
				case EH_WND_DOWN:
				case EH_WND_LEFT:
					g_Context.MoveWindow(EGridDirection(lParam-EH_WND_UP));
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
					goto ignoreMsg;
			}
			break;
		default:
			if(message == g_Context.GetShellMsgID())
			{
				switch(wParam)
				{
				case HSHELL_WINDOWCREATED:
					break;
				case HSHELL_WINDOWDESTROYED:
					break;
				case HSHELL_WINDOWACTIVATED:
				case HSHELL_RUDEAPPACTIVATED:
					break;
				default:
					goto ignoreMsg;
				}
				break;
			}
			ignoreMsg:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
