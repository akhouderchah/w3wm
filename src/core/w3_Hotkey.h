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

struct HotkeyDef
{
	UINT16 m_HotkeyID;

	UINT8 m_MainKey;
	UINT8 m_Modifiers;
};

/**
 * Hotkey Definitions
 */
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
	f(W3WM_RESTART, 'R', EM_WIN | EM_SHIFT) \
	f(W3WM_RESTART_PARTIAL, 'R', EM_WIN) \
	f(W3WM_LOCK, VK_HOME, EM_WIN) \
	f(W3WM_OPEN_CONSOLE, VK_RETURN, EM_WIN)

#define HOTKEYS_AS_ENUM(name, defaultKey, defaultMod) \
	EH_ ## name,

enum EHotkeys
{
	EH_NULL = 0,
	HOTKEYS(HOTKEYS_AS_ENUM)
};

#define HOTKEYS_AS_COUNT(name, defaultKey, defaultMod) \
	1 +

#define HOTKEYS_AS_ACTIVE_COUNT(name, defaultKey, defaultMod) \
	(defaultKey != 0) +

#define HOTKEYS_AS_ARR(name, defaultKey, defaultMod) \
	{EH_ ## name, defaultKey, defaultMod},

#define STR(x) #x
#define HOTKEYS_AS_NAME_ARR(name, defaultKey, defaultMod) \
	_T(STR(name)),

/**
 * Key definitions
 */
#define KEY_NAMES(f)			\
	f("BACKSPACE", VK_BACK)			\
	f("TAB", VK_TAB)			\
	f("ENTER", VK_RETURN)		\
	f("MENU", VK_MENU)			\
	f("PAUSE", VK_PAUSE)		\
	f("CAPS", VK_CAPITAL)		\
	f("ESCAPE", VK_ESCAPE)		\
	f("SPACE", VK_SPACE)		\
	f("PAGEUP", VK_PRIOR)		\
	f("PAGEDOWN", VK_NEXT)		\
	f("END", VK_END)			\
	f("HOME", VK_HOME)			\
	f("LEFT", VK_LEFT)			\
	f("UP", VK_UP)				\
	f("RIGHT", VK_RIGHT)		\
	f("DOWN", VK_DOWN)			\
	f("SELECT", VK_SELECT)		\
	f("PRINT", VK_PRINT)		\
	f("PRINTSCREEN", VK_SNAPSHOT)\
	f("INSERT", VK_INSERT)		\
	f("DELETE", VK_DELETE)		\
	f("F1", VK_F1)				\
	f("F2", VK_F2)				\
	f("F3", VK_F3)				\
	f("F4", VK_F4)				\
	f("F5", VK_F5)				\
	f("F6", VK_F6)				\
	f("F7", VK_F7)				\
	f("F8", VK_F8)				\
	f("F9", VK_F9)				\
	f("F10", VK_F10)			\
	f("F11", VK_F11)			\
	f("F12", VK_F12)

#define KEYS_AS_PAIRS(a, b) {a, {b, 0}},
