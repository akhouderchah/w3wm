#include "w3_Context.h"
#include "w3_Hotkey.h"

#include <tchar.h>
#include <windows.h>

w3Context g_Context;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow)
{
	if(!g_Context.Initialize(hInstance))
	{
		RELEASE_MESSAGE(_T("Error"), _T("Failed to initialize!"));
		return 1;
	}

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
			g_Context.Shutdown();
			PostQuitMessage(WM_QUIT);
			break;
		case WM_HOTKEY:
			switch(lParam)
			{
				case EH_WND_OPEN:
					g_Context.OpenWindow(_T("notepad"));
					break;
				case EH_WND_CLOSE:
					g_Context.CloseWindow();
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
				case EH_WND_FULLSCREEN:
					g_Context.ToggleFullscreen();
					break;
				case EH_W3WM_CLOSE:
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
		case WM_ICON_CALLBACK:
			switch(lParam)
			{
				case WM_RBUTTONDOWN:
				{
					// Create menu
					HMENU popMenu = CreatePopupMenu();
					AppendMenu(popMenu, MF_STRING, ID_MENU_EXIT, "Exit");

					// Track menu
					POINT pCursor;
					GetCursorPos(&pCursor);
					SetForegroundWindow(hWnd);
					TrackPopupMenu(popMenu, TPM_LEFTBUTTON | TPM_RIGHTALIGN, pCursor.x, pCursor.y,
						0, hWnd, NULL);

					break;
				}
				default:
					goto ignoreMsg;
			}
			break;
		case WM_COMMAND:
		{
			WORD id = LOWORD(wParam);
			WORD event = HIWORD(wParam);

			switch(id)
			{
			case ID_MENU_EXIT:
				PostMessage(hWnd, WM_DESTROY, 0, 0);
				break;
			default:
				goto ignoreMsg;
			}
			break;
		}
		default:
			if(message == g_Context.GetShellMsgID() && g_Context.IsReady())
			{
				switch(wParam)
				{
				case HSHELL_WINDOWCREATED:
				{
					HWND wnd = (HWND)lParam;
					if(g_Context.IsRelevantWindow(wnd))
					{
						g_Context.TrackWindow(wnd);
					}
					break;
				}
				case HSHELL_WINDOWDESTROYED:
				{
					// Note: We don't check if the window is relevant,
					// since a destroyed window might no longer satisfy
					// the criteria of a relevant window
					g_Context.UntrackWindow((HWND)lParam);
					break;
				}
				case HSHELL_WINDOWACTIVATED:
				case HSHELL_RUDEAPPACTIVATED:
				{
					HWND wnd = (HWND)lParam;
					if(g_Context.IsRelevantWindow(wnd))
					{
						g_Context.NotifyActivation(wnd);
					}
					break;
				}
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
