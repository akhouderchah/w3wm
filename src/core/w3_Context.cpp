#include "w3_Context.h"
#include "w3_Core.h"
#include "w3_DLL.h"

// TODO REMOVE
#include "w3_Window.h"

#include <windows.h>
#include "Shlwapi.h"
#include <vector>

extern LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
BOOL CALLBACK EnumWindowProc_Register(HWND hwnd, LPARAM lParam);

std::vector<MonitorInfo> g_SecondaryMonitors;
MonitorInfo g_PrimaryMonitor;
bool g_IsPrimarySet = false;

// TODO REMOVE
WindowGrid *pGridTest;

#define ADD_BLACKLIST(str) \
	m_ClassBlacklist.insert(str)

#define ADD_BLACKLIST_PREFIX(str) \
	m_ClassBlacklist.insert(str); \
	m_PrefixLengths.insert((sizeof(str)-sizeof(TCHAR))/sizeof(TCHAR))

w3Context::w3Context() :
	m_HUserDLL(NULL),
	m_ShellMsgID(0),
	m_IsInitialized(false)
{}

bool w3Context::Initialize(HINSTANCE hInstance)
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
		MessageBoxEx(NULL, _T("w3wm failed to register the window class"), T_ERROR_TITLE, MB_OK | MB_ICONERROR, 0);
		return false;
	}

	m_Hwnd = CreateWindowEx(0, T_WNDCLASS_NAME, T_APP_NAME, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
	if(!m_Hwnd)
	{
		MessageBoxEx(NULL, _T("w3wm failed to create the window"), T_ERROR_TITLE, MB_OK | MB_ICONERROR, 0);
		return false;
	}

	m_HUserDLL = GetModuleHandle(_T("USER32.DLL"));
	if(!m_HUserDLL)
	{
		MessageBoxEx(NULL, _T("w3wm failed to get USER32.DLL"), T_ERROR_TITLE, MB_OK | MB_ICONERROR, 0);
		return false;
	}

	BOOL (__stdcall *RegisterShellHookWindowFunc)(HWND) =
		(BOOL (__stdcall *)(HWND))GetProcAddress(m_HUserDLL, "RegisterShellHookWindow");
	if(!RegisterShellHookWindowFunc)
	{
		MessageBoxEx(NULL, _T("w3wm failed to get RegisterShellHookWindow"), T_ERROR_TITLE, MB_OK | MB_ICONERROR, 0);
		return false;
	}

	RegisterShellHookWindowFunc(m_Hwnd);
	m_ShellMsgID = RegisterWindowMessage(_T("SHELLHOOK"));
	if(!m_ShellMsgID)
	{
		MessageBoxEx(NULL, _T("w3wm failed to get the shell hook message ID"), T_ERROR_TITLE, MB_OK | MB_ICONERROR, 0);
		return false;
	}

	InstallHooks(m_Hwnd);

	// Read whether or not the workstation can lock at startup
	m_InitialLockEnabled = CanWorkstationLock();

	return Start();
}

void w3Context::Shutdown()
{
	// Un-clip cursor
	ClipCursor(0);

	// Remove the injected DLL
	RemoveHooks();

	// Set the original workstation lock enable/disable value
	AllowWorkstationLock(m_InitialLockEnabled);
}

bool w3Context::Restart()
{
	// Clear state that will be replaced after calling Start()
	m_Monitors.Clear();
	m_ClassBlacklist.clear();
	g_IsPrimarySet = false;

	return Start();
}

void w3Context::LockScreen()
{
	if(!AllowWorkstationLock(true))
	{
		DEBUG_MESSAGE(_T("Warning"), _T("Failed to enable workspace lock"));
	}
	LockWorkStation();
}

void w3Context::OpenConsole()
{
	WinExec(m_CmdPath, SW_RESTORE);
}

bool w3Context::MoveFocus(EGridDirection direction, bool bWrapAround)
{
	AllowWorkstationLock(false);

	// Remove and re-install hooks so that key callback is at the top of the chain
	RemoveHooks();
	bool retVal = pGridTest->MoveFocus(direction, bWrapAround);
	InstallHooks(m_Hwnd);

	return retVal;
}

bool w3Context::MoveWindow(EGridDirection direction, bool bWrapAround)
{
	AllowWorkstationLock(false);

	// Remove and re-install hooks so that key callback is at the top of the chain
	RemoveHooks();
	bool retVal = pGridTest->MoveWindow(direction, bWrapAround);
	InstallHooks(m_Hwnd);

	return retVal;
}

bool w3Context::Start()
{
	// Load settings from ini
	TCHAR iniDir[512];
	GetCurrentDirectory(512, iniDir);
	_tcscat_s(iniDir, 100, _T("\\config.ini"));

	if(UpdateHotkeys(iniDir))
	{
		// Get cmd path from ini
		GetPrivateProfileString(_T("Applications"), _T("Cmd"), _T("C:\\Windows\\System32\\cmd.exe"), m_CmdPath, 512, iniDir);
	}

	// Set up window exclusion by class name
	SetupBlacklist();

	// Get monitor scaling (GetScaleFactorForMonitor is only available in Windows 8+)
	// This scaling factor will only be necessary for applications that are not DPI aware
	HWND wnd = 0;
	HDC hDC = GetDC(wnd);
	float scale = float(GetDeviceCaps(hDC, DESKTOPHORZRES)) / GetDeviceCaps(hDC, HORZRES);
	ReleaseDC(wnd, hDC);

	// Get monitors
	if(!EnumDisplayMonitors(NULL, NULL, MonitorProc, (LPARAM)&scale))
	{
		MessageBoxEx(NULL, _T("Failed to enumerate monitors!"), T_ERROR_TITLE, MB_OK | MB_ICONERROR, NULL);
		return false;
	}

	// Add monitors to m_Monitors if we found the primary monitor
	if(!g_IsPrimarySet)
	{
		MessageBoxEx(NULL, _T("Failed to find a primary monitor!"), T_ERROR_TITLE, MB_OK | MB_ICONERROR, 0);
		return false;
	}

	m_Monitors.Insert(std::move(g_PrimaryMonitor));
	for(MonitorInfo &info : g_SecondaryMonitors)
	{
		m_Monitors.Insert(std::move(info));
	}
	g_SecondaryMonitors.clear();

	// Get windows
	EnumWindows(EnumWindowProc_Register, (LPARAM)this);

	pGridTest->Apply();

	pGridTest->FocusCurrent();

	return true;
}

bool w3Context::UpdateHotkeys(PTCHAR iniDir)
{
	// Initialize hotkeys with the defaults
	HotkeyDef defs[] = { HOTKEYS(F_HOTKEY_ARR) };

	// Modify defaults with the ini
	LPCTSTR names[] = { HOTKEYS(F_HOTKEY_NAME_ARR) };
	int max = ARR_SIZE(names);

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

	return (max != 0);
}

void w3Context::SetupBlacklist()
{
	ADD_BLACKLIST(_T("Shell_TrayWnd"));
	ADD_BLACKLIST(_T("Progman"));
	ADD_BLACKLIST(_T("WorkerW"));
	ADD_BLACKLIST(_T("#32770"));					// Annoying Win10 upgrade prompt

	ADD_BLACKLIST(_T("FencesCustomScrollbar"));
	ADD_BLACKLIST(_T("VisualStudioGlowWindow"));

	ADD_BLACKLIST_PREFIX(_T("TaskbarWindow"));
}

bool w3Context::AllowWorkstationLock(bool value)
{
	// Get registry key for DisableLockWorkstation
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"), 0,
		KEY_ALL_ACCESS, &hKey);

	// Set DisableLockWorkstation value
	DWORD d = !value;
	LONG result = RegSetValueEx(hKey, _T("DisableLockWorkstation"), 0, REG_DWORD, (const BYTE*)&d, sizeof(DWORD));

	// Release registry key for DisableLockWorkstation
	RegCloseKey(hKey);

	return (result == ERROR_SUCCESS);
}

bool w3Context::CanWorkstationLock() const
{
	DWORD d, size = sizeof(d);
	LONG result = RegGetValue(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"),
		_T("DisableLockWorkstation"), RRF_RT_DWORD, NULL, &d, &size);

	return (result != ERROR_SUCCESS) || !d;
}

BOOL CALLBACK MonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	if(lprcMonitor->left == 0 && lprcMonitor->top == 0)
	{
		g_PrimaryMonitor = {hdcMonitor, *lprcMonitor};
		pGridTest = new WindowGrid(lprcMonitor, *(float*)dwData);
		g_IsPrimarySet = true;
	}
	else
	{
		g_SecondaryMonitors.push_back({hdcMonitor, *lprcMonitor});
	}

	return TRUE;
}

bool w3Context::IsRelevantWindow(HWND hwnd)
{
	if(IsWindowVisible(hwnd) && GetParent(hwnd) == 0)
	{
		TCHAR className[256];
		int len = GetClassName(hwnd, className, 256);

		// Check to see if full name is in blacklist
		if(m_ClassBlacklist.find(className) != m_ClassBlacklist.end())
		{
			return false;
		}

		// See if relevant prefixes are in blacklist
		for(auto prefixLen : m_PrefixLengths)
		{
			if(len <= prefixLen) continue;

			TCHAR prev = className[prefixLen];
			className[prefixLen] = 0;

			if(m_ClassBlacklist.find(className) != m_ClassBlacklist.end())
			{
				return false;
			}

			className[prefixLen] = prev;
		}

		RECT r;
		GetWindowRect(hwnd, &r);
		DEBUG_MESSAGE(_T("WindowPos"), _T("%s: (%d, %d) -> (%d, %d)"), className,
			r.left, r.top, r.right, r.bottom);

		return true;
	}

	return false;
}

BOOL CALLBACK EnumWindowProc_Register(HWND hwnd, LPARAM lParam)
{
	if(((w3Context*)lParam)->IsRelevantWindow(hwnd))
	{
		pGridTest->Insert(hwnd);
	}

	return TRUE;
}
