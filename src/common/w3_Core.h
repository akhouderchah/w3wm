#pragma once

#include <tchar.h>
#include <atlstr.h>

#define T_WNDCLASS_NAME _T("w3wm_class")
#define T_APP_NAME _T("w3wm")

#define T_ERROR_TITLE _T("w3wm Error")

#define ARR_SIZE(arr) sizeof(arr)/sizeof(arr[0])

#ifdef NDEBUG
#define DEBUG_MESSAGE(msgTitle, format, ...)
#else
#define DEBUG_MESSAGE(msgTitle, format, ...)					\
{																\
	CString msg;												\
	msg.Format(format, __VA_ARGS__);							\
	MessageBoxEx(NULL, msg.GetString(), msgTitle, MB_OK, NULL);	\
}
#endif

enum EGridDirection
{
	EGD_UP,
	EGD_RIGHT,
	EGD_DOWN,
	EGD_LEFT,
	EGD_COUNT
};
