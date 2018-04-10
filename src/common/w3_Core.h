#pragma once

#include <tchar.h>
#include <atlstr.h>

/* w3 Version */
#define W3_VERSION_MAJOR 0
#define W3_VERSION_MINOR 2

 /* w3 Names */
#define T_WNDCLASS_NAME _T("w3wm_class")
#define T_APP_NAME _T("w3wm")

#define T_ERROR_TITLE _T("w3wm Error")

/* w3 Constants */
#define WM_ICON_CALLBACK WM_USER+1
#define WM_STUBCOMM WM_USER+10

#define ID_MENU_EXIT 1

enum StubMessages
{
	ESM_INJECT_DLL = 1,
	ESM_WITHDRAW_DLL,
	ESM_UPDATE_HOTKEYS,
};

enum EGridDirection
{
	EGD_UP,
	EGD_RIGHT,
	EGD_DOWN,
	EGD_LEFT,
	EGD_COUNT
};

/* w3 Macros */
#define ARR_SIZE(arr) sizeof(arr)/sizeof(arr[0])

#define RELEASE_MESSAGE(msgTitle, format, ...)					\
{																\
	CString msg, title;											\
	msg.Format(format, __VA_ARGS__);							\
	title.Format("w3wm v%d.%02d - %s", W3_VERSION_MAJOR,		\
		W3_VERSION_MINOR, msgTitle);							\
	MessageBoxEx(NULL, msg.GetString(), title, MB_OK, NULL);	\
}

#ifdef NDEBUG
#define DEBUG_MESSAGE(msgTitle, format, ...)
#else
#define DEBUG_MESSAGE(msgTitle, format, ...)					\
	RELEASE_MESSAGE(msgTitle, format, __VA_ARGS__);
#endif

struct MonitorInfo
{
	HMONITOR m_Hmonitor;
	RECT m_ScreenBounds;
	float m_DpiScaling;
	size_t m_WorkspaceIndex;
};
