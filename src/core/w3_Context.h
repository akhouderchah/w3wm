#pragma once

#include <windows.h>
#include "w3_Monitor.h"
#include "w3_Window.h"

#include <unordered_set>

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
	 * @brief Close the current window
	 * @return true if a window was closed, false otherwise
	 */
	bool CloseWindow();

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
	 * @brief Return whether or not w3wm should track the given window
	 */
	bool IsRelevantWindow(HWND hwnd);

	/**
	 * @brief Return the msg ID for HSHELL_* messages
	 */
	inline UINT GetShellMsgID() const{ return m_ShellMsgID; }

private:
	bool Start();
	bool UpdateHotkeys(PTCHAR iniDir);

	/**
	 * @brief Initialize the window class blacklist
	 */
	void SetupBlacklist();

	/**
	 * @brief Enable/disable workstation locking
	 *
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

	UINT m_ShellMsgID;

	bool m_IsInitialized;
	bool m_InitialLockEnabled;

	TCHAR m_CmdPath[256];

	std::unordered_set<std::basic_string<TCHAR>> m_ClassBlacklist;
	std::unordered_set<int> m_PrefixLengths;

	friend BOOL CALLBACK MonitorProc(HMONITOR,HDC,LPRECT,LPARAM);
	friend BOOL CALLBACK EnumWindowProc_Register(HWND,LPARAM);
	static MonitorGrid s_Monitors;
	static std::vector<WindowGrid> s_Workspaces;
	static size_t s_ActiveWorkspace;
};
