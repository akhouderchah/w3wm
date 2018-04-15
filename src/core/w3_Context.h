#pragma once

#include <windows.h>
#include "w3_Monitor.h"
#include "w3_Window.h"

#include <unordered_set>
#include <unordered_map>

/**
 * @brief The top-level class for w3wm
 *
 * Provides the top-level interface, controls the lifetime of the
 * w3wm window, and processes the message queue.
 */
class w3Context
{
public:
	w3Context();

	/**
	 * @brief Perform w3wm initialization
	 * @return true on success, false otherwise
	 */
	bool Initialize(HINSTANCE hInstance);
	/**
	 * @brief Perform w3wm Shutdown
	 */
	void Shutdown();

	/**
	 * @brief Re-start w3wm with a "clean slate"
	 * @return true on success, false otherwise
	 */
	bool Restart();

	/**
	 * @brief Locks the workstation
	 */
	void LockScreen();
	/**
	 * @brief Opens the console
	 * @note The console actually opened depends on whether or
	 *       not the provided config file overrides the default
	 *       console application. By default, cmd.exe is opened.
	 */
	void OpenConsole();

	/**
	 * @brief Move the window focus to the provided direction
	 */
	bool MoveFocus(EGridDirection direction, bool bWrapAround=true);

	/**
	 * @brief Move the current window towards the provided direction
	 */
	bool MoveWindow(EGridDirection direction, bool bWrapAround=true);

	/**
	 * @brief Attempts to start the specified program
	 */
	bool OpenWindow(LPCTSTR filename);

	/**
	 * @brief Close the current window
	 * @return true if a window was closed, false otherwise
	 */
	bool CloseWindow();

	/**
	 * @brief Toggle whether the current window is fullscreened
	 */
	void ToggleFullscreen();

	/**
	 * @brief Track window in the proper workspace
	 * @return Whether or not the window is now being tracked
	 */
	bool TrackWindow(HWND wnd);

	/**
	 * @brief Find and untrack window
	 * @return true if window was found, false otherwise
	 */
	bool UntrackWindow(HWND wnd);

	/**
	 * @brief Notifies w3wm about a window activation
	 *
	 * In the case that this activation was brought about by a focus by
	 * w3wm, the PendingFocus flag will simply be unset. If this activation
	 * was brought about by the user manually clicking on a window, this will
	 * update the internal data structures so that the current window is the
	 * wnd (if wnd is a relevant window).
	 */
	void NotifyActivation(HWND wnd);

	/**
	 * @brief Return whether or not w3wm should track the given window
	 */
	bool IsRelevantWindow(HWND hwnd);

	/**
	 * @brief Returns whether or not w3wm has been properly started
	 */
	inline bool IsReady() const{ return m_IsReady; }

	/**
	 * @brief Return the msg ID for HSHELL_* messages
	 */
	inline UINT GetShellMsgID() const{ return m_ShellMsgID; }

private:
	bool Start();
	bool UpdateHotkeys(TCHAR *iniDir);

	using VirtualKeyMap = std::unordered_map<std::string, UINT8>;
	/**
	 * @brief Set pHotkey based on passed in string
	 */
	void ParseHotkey(LPTSTR str, const VirtualKeyMap &keyMap, struct HotkeyDef *pHotkey) const;

	/**
	 * @brief Creates the default ini with the given path
	 */
	bool GenerateDefaultIni(LPCTSTR filename) const;

	/**
	 * @brief Perform DLL injection
	 * @note On 64-bit machines, this also takes care of signaling the
	 *       stub to perform injection.
	 */
	void InjectDLL();

	/**
	 * @brief Remove injected DLL
	 * @note On 64-bit machines, this also takes care of signaling the
	 *       stub to perform injection.
	 */
	void WithdrawDLL();

	/**
	 * @brief Start the 64-bit stub
	 */
	bool Execute64Bit();

	/**
	 * @brief Initialize the window class blacklist
	 */
	void SetupBlacklist();

	/**
	 * @brief Create user tokens for opening new applications
	 * @note This currently only creates a low integrity level token.
	 *       In the future, additional tokens may be created for running
	 *       applications as administrator, etc.
	 */
	bool SetupTokens();

	/**
	 * @brief Enable/disable workstation locking
	 * @note For this method to succeed, w3wm must be run with
	 *       administrator privileges. Otherwise, either Win+L
	 *       will cause workstation locks, or workstation locking
	 *       will not work at all.
	 */
	bool AllowWorkstationLock(bool value);

	/**
	 * @brief Read the registry to determine if workstation can lock
	 */
	bool CanWorkstationLock() const;

	/**
	 * @brief w3 representation of an HWND's location
	 */
	struct WindowCoord
	{
		inline bool IsValid() const{ return m_WorkspaceIndex != -1l; }
		static WindowCoord CreateNull(){ return {-1l, 0, 0}; }

		size_t m_WorkspaceIndex;
		size_t m_Column;
		size_t m_Row;
	};

	/**
	 * @brief Find the provided window
	 */
	WindowCoord FindWindow(HWND hwnd) const;

	inline WindowGrid &GetWorkspace() const{
		assert(s_ActiveWorkspace < s_Workspaces.size());
		return s_Workspaces[s_ActiveWorkspace];
	}

private:
	HWND m_Hwnd;
	HMODULE m_HUserDLL;
	HANDLE m_LowIntegrityToken;

	UINT m_ShellMsgID;

	// Notification icon data
	NOTIFYICONDATA m_IconData;

	// Pipe handles for communication with stub process
	HANDLE m_hStub_InRead;
	HANDLE m_hStub_InWrite;
	HANDLE m_hStub_OutRead;
	HANDLE m_hStub_OutWrite;
	HWND m_hStubWnd;

	// TODO make into bitflag
	bool m_IsInitialized;
	bool m_IsReady;
	bool m_InitialLockEnabled;
	bool m_PendingFocus; // did w3wm make a focus that hasn't yet been notified

	TCHAR m_CmdPath[256];

	std::unordered_set<std::basic_string<TCHAR>> m_ClassBlacklist;
	std::unordered_set<int> m_PrefixLengths;

	friend BOOL CALLBACK MonitorProc(HMONITOR,HDC,LPRECT,LPARAM);
	friend BOOL CALLBACK EnumWindowProc_Register(HWND,LPARAM);
	static MonitorGrid s_Monitors;
	static std::vector<WindowGrid> s_Workspaces;
	static size_t s_ActiveWorkspace;
};
