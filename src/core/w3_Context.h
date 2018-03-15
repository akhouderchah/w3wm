#pragma once

#include <windows.h>
#include "w3_Monitor.h"

#include <unordered_set>

class w3Context
{
public:
	w3Context();

	bool Initialize(HINSTANCE hInstance);
	void Shutdown();

	bool Restart();

	void OpenConsole();
	void LockScreen();

	inline UINT GetShellMsgID() const{ return m_ShellMsgID; }

	bool IsRelevantWindow(HWND hwnd);

private:
	bool Start();
	bool UpdateHotkeys(PTCHAR iniDir);

	void SetupBlacklist();

private:
	HWND m_Hwnd;
	HMODULE m_HUserDLL;
	MonitorGrid m_Monitors;

	UINT m_ShellMsgID;

	bool m_IsInitialized;

	TCHAR m_CmdPath[256];

	std::unordered_set<std::basic_string<TCHAR>> m_ClassBlacklist;
	std::unordered_set<int> m_PrefixLengths;
};
