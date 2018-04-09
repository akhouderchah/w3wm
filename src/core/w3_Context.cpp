#include "w3_Context.h"
#include "w3_Core.h"
#include "w3_DLL.h"
#include "resource.h"

#include <windows.h>
#include <sddl.h>
#include "Shlwapi.h"
#include <vector>

using IsWow64Process_t = BOOL (WINAPI *)(HANDLE, PBOOL);

extern LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
BOOL CALLBACK EnumWindowProc_Register(HWND hwnd, LPARAM lParam);

MonitorGrid w3Context::s_Monitors;
std::vector<WindowGrid> w3Context::s_Workspaces;
size_t w3Context::s_ActiveWorkspace = 0;

#define ADD_BLACKLIST(str) \
	m_ClassBlacklist.insert(str)

#define ADD_BLACKLIST_PREFIX(str) \
	m_ClassBlacklist.insert(str); \
	m_PrefixLengths.insert((sizeof(str)-sizeof(TCHAR))/sizeof(TCHAR))

w3Context::w3Context() :
	m_HUserDLL(NULL),
	m_ShellMsgID(0),
	m_hStub_InRead(0), m_hStub_InWrite(0),
	m_hStub_OutRead(0), m_hStub_OutWrite(0),
	m_hStubWnd(0),
	m_IsInitialized(false),
	m_PendingFocus(false)
{}

bool w3Context::Initialize(HINSTANCE hInstance)
{
	if(m_IsInitialized){ return Start(); }

	// Create w3 window with null style
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

	m_Hwnd = CreateWindowEx(0, T_WNDCLASS_NAME, T_APP_NAME, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
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

	// Setup window to get shell messages
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

	// Setup user token
	if(!SetupTokens())
	{
		RELEASE_MESSAGE(_T("Error"), _T("Failed to get the user tokens"));
		return false;
	}

	// Spawn 64-bit stub process if we are on 64-bit windows
	HMODULE hKernel32 = GetModuleHandle(_T("kernel32"));
	if(!hKernel32)
	{
		RELEASE_MESSAGE("Error", "Failed to get Kernel32.dll");
		return false;
	}

	IsWow64Process_t fnIsWow64Process = (IsWow64Process_t)GetProcAddress(hKernel32, "IsWow64Process");
	if(!fnIsWow64Process)
	{
		RELEASE_MESSAGE("Error", "Failed to get IsWow64Process from Kernel32.DLL.");
		return false;
	}

	BOOL isSystem64Bit = FALSE;
	fnIsWow64Process(GetCurrentProcess(), &isSystem64Bit);
	if(isSystem64Bit)
	{
		if(!Execute64Bit())
		{
			RELEASE_MESSAGE("Warning", "Failed to start 64-bit stub.\n"
				"w3wm may not behave as expected with all windows");
		}
		else
		{
			// Wait to get HWND from the stub
			unsigned long h;
			DWORD readSize;
			if(!ReadFile(m_hStub_OutRead, &h, sizeof(h), &readSize, NULL) ||
				readSize != sizeof(h) || h == 0)
			{
				RELEASE_MESSAGE("Error", "Failed to get the window handle from the stub.");
				return false;
			}

			m_hStubWnd = (HWND)h;
		}
	}

	// Inject DLL
	InjectDLL();

	// Read whether or not the workstation can lock at startup
	m_InitialLockEnabled = CanWorkstationLock();

	// Add icon to notification area
	m_IconData.cbSize = sizeof(NOTIFYICONDATA);
	m_IconData.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	m_IconData.uCallbackMessage = WM_USER + 1;
	m_IconData.hWnd = m_Hwnd;
	m_IconData.uID = WM_USER + 2;
	m_IconData.uVersion = NOTIFYICON_VERSION;
	m_IconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
	_tcscpy_s(m_IconData.szTip, _T("w3wm"));

	if(!Shell_NotifyIcon(NIM_ADD, &m_IconData) || !Shell_NotifyIcon(NIM_SETVERSION, &m_IconData))
	{
		RELEASE_MESSAGE("Warning", "Failed to add icon to notification area.");
	}

	m_IsInitialized = true;
	return Start();
}

void w3Context::Shutdown()
{
	if(!m_IsInitialized){ return; }

	// Un-clip cursor
	ClipCursor(0);

	// Close security tokens
	CloseHandle(m_LowIntegrityToken);

	// Shut down stub if it exists, and close pipe handles
	if(m_hStubWnd)
	{
		CloseHandle(m_hStub_InRead);
		CloseHandle(m_hStub_InWrite);
		CloseHandle(m_hStub_OutRead);
		CloseHandle(m_hStub_OutWrite);
		PostMessage(m_hStubWnd, WM_DESTROY, 0, 0);
		m_hStubWnd = 0;
	}

	// Remove the injected DLL
	WithdrawDLL();

	// Set the original workstation lock enable/disable value
	AllowWorkstationLock(m_InitialLockEnabled);

	// Remove notification icon
	Shell_NotifyIcon(NIM_DELETE, &m_IconData);

	m_IsInitialized = false;
}

bool w3Context::Restart()
{
	// Clear state that will be replaced after calling Start()
	s_Monitors.Clear();
	m_ClassBlacklist.clear();

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
	WithdrawDLL();
	bool retVal = GetWorkspace().MoveFocus(direction, false);

	// If window is at edge, try to move monitors
	if(!retVal)
	{
		retVal = s_Monitors.Move(direction, bWrapAround);

		if(retVal)
		{
			// If in new monitor, focus in its workspace
			s_ActiveWorkspace = s_Monitors.GetWorkspaceIndex();

			GetWorkspace().MoveToEdgeFrom(direction);
			retVal = GetWorkspace().FocusCurrent();
		}
		else if(bWrapAround)
		{
			// If no new monitor, wrap on single workspace if needed
			retVal = GetWorkspace().MoveFocus(direction, bWrapAround);
		}
	}

	InjectDLL();

	m_PendingFocus = retVal;
	return retVal;
}

bool w3Context::MoveWindow(EGridDirection direction, bool bWrapAround)
{
	AllowWorkstationLock(false);

	// Remove and re-install hooks so that key callback is at the top of the chain
	WithdrawDLL();
	bool retVal = GetWorkspace().MoveWindow(direction, false);

	// If window is at edge, try to move monitors
	if(!retVal)
	{
		retVal = s_Monitors.Move(direction, bWrapAround);
		if(retVal)
		{
			// Set workspace to that of the new monitor
			size_t newWorkspace = s_Monitors.GetWorkspaceIndex();

			// Insert window to this workspace
			retVal = s_Workspaces[newWorkspace].Insert(GetWorkspace().GetCurrent(), direction);

			// Remove window from the old workspace
			GetWorkspace().RemoveCurrent();
			GetWorkspace().Apply();

			// Update to new workspace
			s_ActiveWorkspace = newWorkspace;
			GetWorkspace().Apply();
			GetWorkspace().MoveToEdgeFrom(direction);
		}
		else if(bWrapAround)
		{
			// If no new monitor and wrapping, just wrap on single workspace
			retVal = GetWorkspace().MoveWindow(direction, bWrapAround);
		}
	}
	InjectDLL();

	GetWorkspace().FocusCurrent();

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

	// Ensure the primary monitor was added to the MonitorGrid
	if(!s_Monitors.HasPrimary())
	{
		MessageBoxEx(NULL, _T("Failed to find a primary monitor!"), T_ERROR_TITLE, MB_OK | MB_ICONERROR, 0);
		s_Monitors.Clear();
		return false;
	}

	// Set active workspace to that of the first-enumerated monitor
	// TODO perhaps set this instead to the primary monitor?
	s_ActiveWorkspace = 0;
	if(!s_Monitors.MoveToWorkspace(s_ActiveWorkspace))
	{
		RELEASE_MESSAGE(_T("Error"), _T("MonitorGrid failed to find the active workspace!"));
		return false;
	}

	// Get windows
	EnumWindows(EnumWindowProc_Register, (LPARAM)this);

	// Apply all workspaces
	for(auto &workspace : s_Workspaces)
	{
		workspace.Apply();
	}

	GetWorkspace().FocusCurrent();

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
		DEBUG_MESSAGE("Warning", "Ini not found - using default properties");
	}
	else
	{
		for(int i = 0; i < max; ++i)
		{
			TCHAR inBuf[80];
			DWORD res = GetPrivateProfileString(_T("Keybindings"), names[i], _T(""), inBuf, 80, iniDir);

			if(res != 0)
			{
				ParseHotkey(&defs[i], inBuf);
			}
		}
	}

	// Inform 32-bit DLL of hotkey mappings
	SetHotkeys(defs, ARR_SIZE(defs));

	// If it exists, send hotkey mappings to 64-bit stub
	if(m_hStubWnd)
	{
		// Send size of defs array
		DWORD bytesWritten;
		DWORD defsSize = sizeof(defs);
		DWORD hotkeySize = sizeof(HotkeyDef);
		if(!WriteFile(m_hStub_InWrite, &defsSize, sizeof(DWORD), &bytesWritten, NULL) ||
			bytesWritten != sizeof(DWORD))
		{
			RELEASE_MESSAGE("Warning", "Failed to update 64-bit stub hotkey mappings");
		}
		// Send HotkeyDef size
		else if(!WriteFile(m_hStub_InWrite, &hotkeySize, sizeof(DWORD), &bytesWritten, NULL) ||
			bytesWritten != sizeof(DWORD))
		{

			RELEASE_MESSAGE("Warning", "Failed to update 64-bit stub hotkey mappings");
		}
		// Send defs array
		else if(!WriteFile(m_hStub_InWrite, defs, sizeof(defs), &bytesWritten, NULL) ||
			bytesWritten != sizeof(defs))
		{
			RELEASE_MESSAGE("Warning", "Failed to update 64-bit stub hotkey mappings");
		}
		// Send update hotkey message
		else
		{
			if(!PostMessage(m_hStubWnd, ESM_UPDATE_HOTKEYS, WM_STUBCOMM, 0))
			{
				RELEASE_MESSAGE("Warning", "Failed to send 64-bit stub the update hokey message.");
			}
		}
	}

	return (max != 0);
}

void w3Context::InjectDLL()
{
	// Always perform 32-bit injection
	InstallHooks(m_Hwnd);

	if(m_hStubWnd)
	{
		// Send inject message to stub
		PostMessage(m_hStubWnd, WM_STUBCOMM, ESM_INJECT_DLL, 0);
	}
}

void w3Context::WithdrawDLL()
{
	// Always remove 32-bit DLL
	RemoveHooks();

	if(m_hStubWnd)
	{
		// Send withdraw message to stub
		PostMessage(m_hStubWnd, WM_STUBCOMM, ESM_WITHDRAW_DLL, 0);
	}
}

bool w3Context::Execute64Bit()
{
	// Pass PID over command line
	CString cmdLine;
	cmdLine.Format(_T("w3_stub.exe %u"), m_Hwnd);

	//// Create STDOUT and STDIN pipes
	// Allow pipes to be inherited by stub
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	// Create STDOUT pipes and only make the write pipe inheritable
	if(!CreatePipe(&m_hStub_OutRead, &m_hStub_OutWrite, &sa, 0))
	{
		return false;
	}
	else if(!SetHandleInformation(m_hStub_OutRead, HANDLE_FLAG_INHERIT, 0))
	{
		CloseHandle(m_hStub_OutRead);
		CloseHandle(m_hStub_OutRead);
		return false;
	}

	// Create STDIN pipes and only make the read pipe inheritable
	if(!CreatePipe(&m_hStub_InRead, &m_hStub_InWrite, &sa, 0))
	{
		CloseHandle(m_hStub_OutRead);
		CloseHandle(m_hStub_OutWrite);
		return false;
	}
	else if(!SetHandleInformation(m_hStub_InWrite, HANDLE_FLAG_INHERIT, 0))
	{
		CloseHandle(m_hStub_OutRead);
		CloseHandle(m_hStub_OutWrite);
		CloseHandle(m_hStub_InRead);
		CloseHandle(m_hStub_InWrite);
		return false;
	}

	PROCESS_INFORMATION procInfo;
	ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdError = m_hStub_OutWrite;
	startupInfo.hStdOutput = m_hStub_OutWrite;
	startupInfo.hStdInput = m_hStub_InRead;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process
	BOOL bSuccess = CreateProcess(NULL, (LPSTR)cmdLine.GetString(),
		NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &procInfo);
	if(!bSuccess)
	{
		return false;
	}

	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
	return true;
}

void w3Context::SetupBlacklist()
{
	//ADD_BLACKLIST(_T("#32770"));					// Windows message boxes
}

bool w3Context::SetupTokens()
{
	HANDLE currToken, baseToken;

	// Get current process token
	if(!OpenProcessToken(GetCurrentProcess(),
		TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY |TOKEN_ASSIGN_PRIMARY,
		&currToken))
	{
		return false;
	}

	// Duplicate with fixed integrity level
	if(!DuplicateTokenEx(currToken, 0, NULL, SecurityImpersonation,
		TokenPrimary, &baseToken))
	{
		CloseHandle(currToken);
		return false;
	}

	// Set duplicate to low integrity level
	const char *strSID = "S-1-16-4096";
	PSID psid = NULL;
	if(!ConvertStringSidToSidA(strSID, &psid))
	{
		CloseHandle(currToken);
		CloseHandle(baseToken);
		return false;
	}

	TOKEN_MANDATORY_LABEL tml;
	ZeroMemory(&tml, sizeof(tml));
	tml.Label.Attributes = SE_GROUP_INTEGRITY;
	tml.Label.Sid = psid;

	if(!SetTokenInformation(baseToken, TokenIntegrityLevel, &tml,
		sizeof(tml) + GetLengthSid(psid)))
	{
		LocalFree(psid);
		CloseHandle(currToken);
		CloseHandle(baseToken);
		return false;
	}

	// Save duplicated token and clean up
	m_LowIntegrityToken = baseToken;
	LocalFree(psid);
	CloseHandle(currToken);
	return true;
}

bool w3Context::AllowWorkstationLock(bool value)
{
	// Get registry key for DisableLockWorkstation
	HKEY hKey;
	RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"), 0,
		NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);

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

w3Context::WindowCoord w3Context::FindWindow(HWND hwnd) const
{
	// TODO: The current implementation simply iterates through all
	// workspaces to find a window. This may prove to be slow,
	// and will probably need to be optimized in the future.
	for(size_t i=0; i < s_Workspaces.size(); ++i)
	{
		size_t col=0, row=0;
		if(s_Workspaces[i].Find(hwnd, &col, &row))
		{
			return {i, col, row};
		}
	}

	return WindowCoord::CreateNull();
}

BOOL CALLBACK MonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	// Track monitor
	MonitorInfo *pMon = w3Context::s_Monitors.Insert(
		{hMonitor, *lprcMonitor, *(float*)dwData, w3Context::s_Workspaces.size()});

	if(pMon)
	{
		// Create workspace for the monitor
		w3Context::s_Workspaces.push_back(WindowGrid());
		w3Context::s_Workspaces.rbegin()->AttachToMonitor(*pMon);
	}

	return TRUE;
}

bool w3Context::OpenWindow(LPCSTR filename)
{
	// Attempt to find the program
	TCHAR pathBuf[512];

	PVOID oldValue = NULL;
	Wow64DisableWow64FsRedirection(&oldValue);

	DWORD len = SearchPath(NULL, filename, _T(".exe"), 512, pathBuf, NULL);
	if(!len)
	{
		RELEASE_MESSAGE(_T("Fu"), _T("Couldn't find %s"), filename);
		return false;
	}
	Wow64RevertWow64FsRedirection(oldValue);
	DEBUG_MESSAGE(_T("Info"), _T("Opening %s"), pathBuf);

	// Run program with low integrity
	// TODO LoadUserProfile might be needed when HKEY_CURRENT_USER registry entries
	//      are used by the spawned process
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if(!CreateProcessAsUser(m_LowIntegrityToken, pathBuf, NULL, NULL,
		NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		return false;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return true;
}

bool w3Context::CloseWindow()
{
	// TODO must change when multi-monitor support is added
	HWND wnd = GetWorkspace().GetCurrent();
	if(wnd != (HWND)0)
	{
		// Note: technically, the window could handle WM_CLOSE
		// in a way that would lead to different behavior than
		// calling DestroyWindow. Since we can't call DestroyWindow
		// from this thread, though, this will hopefully be sufficient.
		return !!::PostMessage(wnd, WM_CLOSE, 0, 0);
	}
	return false;
}

void w3Context::ToggleFullscreen()
{
	WindowGrid &workspace = GetWorkspace();
	workspace.ToggleFullscreen();
	workspace.Apply();
	workspace.FocusCurrent();
}

bool w3Context::TrackWindow(HWND wnd)
{
	bool success = GetWorkspace().Insert(wnd);
	if(success)
	{
		GetWorkspace().Apply();
		GetWorkspace().FocusCurrent();
	}
	return success;
}

bool w3Context::UntrackWindow(HWND wnd)
{
	WindowCoord coord = FindWindow(wnd);

	if(!coord.IsValid())
	{
		return false;
	}

	s_Workspaces[coord.m_WorkspaceIndex].Remove(coord.m_Column, coord.m_Row);
	s_Workspaces[coord.m_WorkspaceIndex].Apply();

	GetWorkspace().FocusCurrent();
	return true;
}

void w3Context::NotifyActivation(HWND wnd)
{
	if(m_PendingFocus)
	{
		m_PendingFocus = false;
		return;
	}

	WindowCoord coord = FindWindow(wnd);
	if(coord.IsValid())
	{
		if(!s_Monitors.MoveToWorkspace(coord.m_WorkspaceIndex))
		{
			DEBUG_MESSAGE(_T("Error"), _T("MonitorGrid failed to find the active workspace!"));
			return;
		}

		s_ActiveWorkspace = coord.m_WorkspaceIndex;
		GetWorkspace().MoveTo(coord.m_Column, coord.m_Row);
	}
}

bool w3Context::IsRelevantWindow(HWND hwnd)
{
	long style = GetWindowLong(hwnd, GWL_EXSTYLE);
	INT16 title;
	if(IsWindowVisible(hwnd) &&
		GetParent(hwnd) == NULL &&
		((GetWindow(hwnd, GW_OWNER) == 0 && !(style & WS_EX_TOOLWINDOW)) ||
		 (GetWindow(hwnd, GW_OWNER) && (style & WS_EX_TOOLWINDOW))) &&
		GetWindowText(hwnd, (char*)&title, 2))
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
	static int i = 0;
	if(((w3Context*)lParam)->IsRelevantWindow(hwnd))
	{
		// Distribute windows roughly evenly between monitors
		w3Context::s_Workspaces[i++ % w3Context::s_Workspaces.size()].Insert(hwnd);
	}

	return TRUE;
}
