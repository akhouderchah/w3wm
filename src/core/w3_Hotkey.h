#pragma once

#include <basetsd.h>
#include <tchar.h>
#include <windows.h>

enum EModifiers
{
	EM_CTRL = 0x01,
	EM_ALT = 0x02,
	EM_WIN = 0x04,
	EM_SHIFT = 0x08
};

typedef struct
{
	UINT16 m_HotkeyID;

	UINT8 m_MainKey;
	UINT8 m_Modifiers;
} HotkeyDef;

void ParseHotkey(HotkeyDef *pHotkey, LPCTSTR str);

#define HOTKEYS(f) \
	f(WND_UP, 'K', EM_WIN | EM_SHIFT) \
	f(WND_RIGHT, 'L', EM_WIN | EM_SHIFT) \
	f(WND_DOWN, 'J', EM_WIN | EM_SHIFT) \
	f(WND_LEFT, 'H', EM_WIN | EM_SHIFT) \
	f(WND_OPEN, 'N', EM_WIN) \
	f(WND_CLOSE, 'D', EM_WIN) \
	f(WND_FULLSCREEN, 'F', EM_WIN) \
	f(FOCUS_UP, 'K', EM_WIN) \
	f(FOCUS_RIGHT, 'L', EM_WIN) \
	f(FOCUS_DOWN, 'J', EM_WIN) \
	f(FOCUS_LEFT, 'H', EM_WIN) \
	f(W3WM_CLOSE, 'Q', EM_WIN) \
	f(W3WM_RESTART, 'R', EM_WIN) \
	f(W3WM_LOCK, VK_HOME, EM_WIN) \
	f(W3WM_OPEN_CONSOLE, VK_RETURN, EM_WIN)

#define F_HOTKEY_ENUM(name, defaultKey, defaultMod) \
	EH_ ## name,

enum EHotkeys
{
	EH_NULL = 0,
	HOTKEYS(F_HOTKEY_ENUM)
};

#define F_HOTKEY_COUNT(name, defaultKey, defaultMod) \
	1 +

#define F_ACTIVE_HOTKEY_COUNT(name, defaultKey, defaultMod) \
	(defaultKey != 0) +

#define F_HOTKEY_ARR(name, defaultKey, defaultMod) \
	{EH_ ## name, defaultKey, defaultMod},

#define STR(x) #x
#define F_HOTKEY_NAME_ARR(name, defaultKey, defaultMod) \
	_T(STR(name)),
