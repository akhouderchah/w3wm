#include "w3_DLL.h"
#include "w3_Core.h"
#include <shellapi.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int)
{
	// Get w3wm main window handle from command line
	int argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if(argc != 2)
	{
		RELEASE_MESSAGE("64-bit stub error", "This program should not be executed manually.\n"
			"w3wm will execute this program if it is needed.");
		LocalFree(argv);
		return 1;
	}

	HWND parentWnd = (HWND)_wtol(argv[1]);
	LocalFree(argv);

	if(parentWnd == 0)
	{
		RELEASE_MESSAGE("64-bit stub error",
			"Failed to get the main window of w3wm");
		return 1;
	}

	// Create minimal window
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
		RELEASE_MESSAGE("Stub Error", "Failed to register the window class.");
		return 1;
	}

	HWND hwnd = CreateWindowEx(0, T_WNDCLASS_NAME, T_APP_NAME, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	if(!hwnd)
	{
		RELEASE_MESSAGE("Stub Error", "Failed to create the window!");
		return 1;
	}

	// Send hwnd to main w3wm process
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hStdout == INVALID_HANDLE_VALUE)
	{
		RELEASE_MESSAGE("Stub Error", "Failed to get the output handle.");
		return 1;
	}

	DWORD bytesWritten;
	if(!WriteFile(hStdout, &hwnd, sizeof(unsigned long), &bytesWritten, NULL))
	{
		RELEASE_MESSAGE("Stub Error", "Failed to send the window handle to the main w3wm process.");
		return 1;
	}

	// Setup message loop
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

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
		// Send lParam to parent
		// TODO
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
