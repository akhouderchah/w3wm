#include "w3_DLL.h"
#include "w3_Core.h"
#include <shellapi.h>

HWND ghParentWnd = 0;
HANDLE ghStdOut = 0;
HANDLE ghStdIn = 0;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool UpdateHotkeys();

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

	ghParentWnd = (HWND)_wtol(argv[1]);
	LocalFree(argv);

	if(ghParentWnd == 0)
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

	// Get stdin and stdout handles
	ghStdIn = GetStdHandle(STD_INPUT_HANDLE);
	if(ghStdIn == INVALID_HANDLE_VALUE)
	{
		RELEASE_MESSAGE("Stub Error", "Failed to get the input handle.");
		return 1;
	}

	ghStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(ghStdOut == INVALID_HANDLE_VALUE)
	{
		RELEASE_MESSAGE("Stub Error", "Failed to get the output handle.");
		CloseHandle(ghStdIn);
		return 1;
	}

	// Send hwnd to main w3wm process
	DWORD bytesWritten;
	if(!WriteFile(ghStdOut, &hwnd, sizeof(unsigned long), &bytesWritten, NULL) ||
		bytesWritten != sizeof(unsigned long))
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
		// Forward message to parent
		PostMessage(ghParentWnd, WM_HOTKEY, wParam, lParam);
		break;
	case WM_STUBCOMM:
		switch(wParam)
		{
		case ESM_INJECT_DLL:
			InstallHooks(hWnd);
			break;
		case ESM_WITHDRAW_DLL:
			RemoveHooks();
			break;
		case ESM_UPDATE_HOTKEYS:
			if(!UpdateHotkeys())
			{
				DEBUG_MESSAGE("Stub Error", "Failed to update the hotkeys");
			}
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

bool UpdateHotkeys()
{
	// Read message size from stdin
	DWORD bytesRead;
	DWORD msgSize;
	if(!ReadFile(ghStdIn, &msgSize, sizeof(DWORD), &bytesRead, NULL) ||
		bytesRead != sizeof(DWORD))
	{
		DEBUG_MESSAGE("Stub Warning", "Failed to read hotkey message size.");
		return false;
	}

	// Read hotkey size from stdin
	DWORD hotkeySize;
	if(!ReadFile(ghStdIn, &hotkeySize, sizeof(DWORD), &bytesRead, NULL) ||
		bytesRead != sizeof(DWORD))
	{
		DEBUG_MESSAGE("Stub Warning", "Failed to read hotkey size.");
		return false;
	}

	// Allocate memory for full message
	HotkeyDef *pDefs = (HotkeyDef*)malloc(msgSize);
	if(!pDefs)
	{
		DEBUG_MESSAGE("Stub Warning", "Failed to allocate memory for hotkey message.");
		return false;
	}

	// Read hotkey message from stdin
	if(!ReadFile(ghStdIn, pDefs, msgSize, &bytesRead, NULL) ||
		bytesRead != msgSize)
	{
		DEBUG_MESSAGE("Stub Warning", "Failed to get hotkey message from stdin.");
		free(pDefs);
		return false;
	}

	// Inform 64-bit DLL of hotkey mappings
	SetHotkeys(pDefs, msgSize/hotkeySize);

	free(pDefs);
	return true;
}
