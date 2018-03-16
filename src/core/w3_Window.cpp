#include "w3_Window.h"
#include <memory>

WindowGrid::WindowGrid(LPRECT pMonitorRect, float dpiScaling) :
	m_DefaultWidthWeight(1), m_MonitorRect(*pMonitorRect),
	m_DpiScaling(dpiScaling)
{
}

bool WindowGrid::Insert(HWND hwnd)
{
	// Make sure window isn't min/max-imized
	ShowWindow(hwnd, SW_RESTORE);

	// Update total width weight
	m_TotalWidthWeight += m_DefaultWidthWeight;

	// Ensure total width weight hasn't overflowed
	if(m_TotalWidthWeight < m_DefaultWidthWeight)
	{
		m_TotalWidthWeight -= m_DefaultWidthWeight;
		return false;
	}

	// Make new column and place window in it
	m_Grid.InsertColumn(m_Grid.ColumnCount(), m_DefaultWidthWeight);
	m_Grid.InsertElement(m_Grid.ColumnCount()-1, 0, {hwnd, 1});
	m_Grid[m_Grid.ColumnCount()-1].m_TotalHeightWeight = 1;

	// Set focus to inserted window
	SetFocus(hwnd);

	return true;
}

void WindowGrid::Apply()
{
	LONG totalWidth = m_MonitorRect.right - m_MonitorRect.left;
	LONG totalHeight = m_MonitorRect.bottom - m_MonitorRect.top;
	float widthUnit = float(totalWidth) / m_TotalWidthWeight;

	auto colCount = m_Grid.ColumnCount();
	decltype(colCount) i;
	LONG currentRight, currentLeft = m_MonitorRect.left;
	for(i = 0; i < colCount-1; ++i)
	{
		// Calculate left/right bounds of windows in this column
		currentRight = currentLeft + LONG(m_Grid[i].m_WidthWeight*widthUnit);

		// Set pos & size of windows in column
	colHandle:
		DEBUG_MESSAGE(_T("Column Dimensions"), _T("Window %d\nleft: %d\nright: %d"),
			m_Grid[i][0].m_Hwnd, currentLeft, currentRight);

		LONG currentBottom, currentTop = m_MonitorRect.top;
		float heightUnit = float(totalHeight) / m_Grid[i].m_TotalHeightWeight;

		// TODO: Need to handle bottom edge-case for rows
		for(auto &node : m_Grid[i])
		{
			// Calculate height of current window
			currentBottom = currentTop + LONG(node.m_HeightWeight*heightUnit);

			// Set pos & size of current window
			float scaling = true ? m_DpiScaling : 1.f; // TODO change true to IsDPIAware
			MoveWindow(node.m_Hwnd,
				currentLeft * scaling,
				currentTop * scaling,
				(currentRight - currentLeft + 1) * scaling,
				(currentBottom - currentTop + 1) * scaling,
				1);

			currentTop = currentBottom;
		}

		currentLeft = currentRight;
	}

	if(i < colCount)
	{
		currentRight = m_MonitorRect.right;
		goto colHandle;
	}
}

bool WindowGrid::MoveFocus(EGridDirection direction, bool bWrapAround)
{
	bool bChanged = m_Grid.Move(direction, bWrapAround);
	if(bChanged)
	{
		Node *pNode = m_Grid.GetCurrent();
		return pNode && FocusWindow(pNode->m_Hwnd);
	}

	return bChanged;
}

void WindowGrid::Clear()
{
	m_Grid.Clear();
	m_TotalWidthWeight = 0;
}

bool WindowGrid::FocusWindow(HWND hwnd)
{
	// Move cursor into window
	RECT r;
	GetWindowRect(hwnd, &r);
	ClipCursor(&r);
	SetCursorPos(r.left + (r.right-r.left)/2, r.top + (r.bottom-r.top)/2);

	// Click cursor down
	INPUT click = {0};
	click.type = INPUT_MOUSE;
	click.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &click, sizeof(INPUT));

	// Click cursor up
	click.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &click, sizeof(INPUT));

	return true;
}
