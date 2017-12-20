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

private:
	bool Start();
	void UpdateHotkeys();

private:
	HWND m_Hwnd;
	MonitorQuadTree m_Monitors;

	bool m_IsInitialized;
};
