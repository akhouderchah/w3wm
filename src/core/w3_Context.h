#pragma once

#include <windows.h>
#include "w3_Monitor.h"

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

	static bool IsRelevantWindow(HWND hwnd);

private:
	bool Start();
	bool UpdateHotkeys(PTCHAR iniDir);

private:
	HWND m_Hwnd;
	HMODULE m_HUserDLL;
	MonitorGrid m_Monitors;

	UINT m_ShellMsgID;

	bool m_IsInitialized;

	TCHAR m_CmdPath[256];
};
