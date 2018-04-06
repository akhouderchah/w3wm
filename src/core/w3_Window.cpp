#include "w3_Window.h"
#include <memory>

bool WindowGrid::s_ShouldClipCursor = false;

WindowGrid::WindowGrid() :
	m_DefaultWidthWeight(1), m_TotalWidthWeight(0),
	m_IsActive(false), m_IsFullscreenMode(false)
{
}

WindowGrid::WindowGrid(WindowGrid &&other) :
	m_Grid(std::move(other.m_Grid)), m_DefaultWidthWeight(other.m_DefaultWidthWeight),
	m_TotalWidthWeight(other.m_TotalWidthWeight), m_MonitorRect(std::move(other.m_MonitorRect)),
	m_DpiScaling(other.m_DpiScaling), m_IsActive(other.m_IsActive),
	m_IsFullscreenMode(other.m_IsFullscreenMode)
{
	other.m_IsActive = false;
}

WindowGrid &WindowGrid::operator=(WindowGrid &&other)
{
	m_Grid = std::move(other.m_Grid);

	m_DefaultWidthWeight = other.m_DefaultWidthWeight;
	m_TotalWidthWeight = other.m_TotalWidthWeight;
	m_MonitorRect = std::move(m_MonitorRect);
	m_DpiScaling = other.m_DpiScaling;

	m_IsActive = other.m_IsActive;
	other.m_IsActive = false;
	m_IsFullscreenMode = other.m_IsFullscreenMode;

	return *this;
}

bool WindowGrid::Insert(HWND hwnd, EGridDirection directionFrom)
{
	assert(EGD_UP <= directionFrom && directionFrom < EGD_COUNT);

	// Make sure window isn't min/max-imized
	ShowWindow(hwnd, SW_RESTORE);

	if(directionFrom & 0x1)
	{
		// Make new right- or left-most column for window
		size_t col = (directionFrom/2) * m_Grid.ColumnCount();
		if(!InsertColumn(col))
		{
			return false;
		}

		// Place window in column
		m_Grid.InsertElement(col, 0, {hwnd, 1});
		m_Grid[col].m_TotalHeightWeight = 1;
	}
	else
	{
		// TODO add vertical direction logic
		return false;
	}

	// Get out of fullscreen mode if insertion is successful
	m_IsFullscreenMode = false;

	return true;
}

void WindowGrid::Remove(size_t col, size_t row)
{
	// If column will be empty, just remove it
	if(m_Grid[col].size() == 1)
	{
		RemoveColumn(col);
		m_Grid.SetColumnIndex(std::min(m_Grid.GetColumnIndex(), m_Grid.ColumnCount()-1));
	}
	else // otherwise, update column weights & remove window
	{
		m_Grid[col].m_TotalHeightWeight -= m_Grid[col][row].m_HeightWeight;
		m_Grid.RemoveElement(col, row);
	}

	// Get out of fullscreen mode on window removal
	m_IsFullscreenMode = false;
}

HWND WindowGrid::GetCurrent()
{
	Node *pNode = m_Grid.GetCurrent();
	if(pNode)
	{
		return pNode->m_Hwnd;
	}
	return (HWND)0;
}

bool WindowGrid::Find(HWND wnd, size_t *pCol, size_t *pRow)
{
	/** The current implementation will simply iterate through all columns to
	 * search for the window. This may be modified in the future to search
	 * based on location, but WindowGrids are currently expected to hold relatively
	 * few windows, so the O(n^2) behavior here should still be acceptable. */

	for(size_t col = 0; col < m_Grid.ColumnCount(); ++col)
	{
		auto &currCol = m_Grid[col];
		for(size_t row = 0; row < currCol.size(); ++row)
		{
			if(currCol[row].m_Hwnd == wnd)
			{
				if(pCol) *pCol = col;
				if(pRow) *pRow = row;
				return true;
			}
		}
	}

	return false;
}

bool WindowGrid::InsertColumn(size_t colPos)
{
	// Update total width weight
	m_TotalWidthWeight += m_DefaultWidthWeight;

	// Ensure total width weight hasn't overflowed
	if(m_TotalWidthWeight < m_DefaultWidthWeight)
	{
		m_TotalWidthWeight -= m_DefaultWidthWeight;
		return false;
	}

	m_Grid.InsertColumn(colPos, m_DefaultWidthWeight);

	return true;
}

void WindowGrid::RemoveColumn(size_t colPos)
{
	assert(colPos < m_Grid.ColumnCount());

	// Update total width weight
	m_TotalWidthWeight -= m_Grid[colPos].m_WidthWeight;

	m_Grid.RemoveColumn(colPos);
}

void WindowGrid::Apply()
{
	if(!m_IsActive || !m_TotalWidthWeight || !m_Grid.ColumnCount()) return;

	LONG totalWidth = m_MonitorRect.right - m_MonitorRect.left;
	LONG totalHeight = m_MonitorRect.bottom - m_MonitorRect.top;
	float widthUnit = float(totalWidth) / m_TotalWidthWeight;

	if(m_IsFullscreenMode)
	{
		// Simply display current window at full size
		float scaling = true ? m_DpiScaling : 1.f; // TODO change true to IsDPIAware
		SetWindowPos(GetCurrent(), HWND_TOP,
			m_MonitorRect.left * scaling,
			m_MonitorRect.top * scaling,
			++totalWidth * scaling,
			++totalHeight * scaling,
			0);
		return;
	}

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
			SetWindowPos(node.m_Hwnd, HWND_TOP,
				currentLeft * scaling,
				currentTop * scaling,
				(currentRight - currentLeft + 1) * scaling,
				(currentBottom - currentTop - 5) * scaling,
				0);

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
	bool bChanged = !m_IsFullscreenMode && m_Grid.Move(direction, bWrapAround);
	if(bChanged)
	{
		Node *pNode = m_Grid.GetCurrent();
		return pNode && FocusWindow(pNode->m_Hwnd);
	}

	return bChanged;
}

bool WindowGrid::MoveWindow(EGridDirection direction, bool bWrapAround)
{
	assert(0 <= direction && direction < EGD_COUNT);

	if(m_Grid.ColumnCount() == 0 || m_IsFullscreenMode){ return false; }

	// If move is horizontal and window is not alone in the column,
	// place into its own column
	if((direction & 0x1) && !m_Grid.IsLoneNode())
	{
		auto col = m_Grid.GetColumnIndex();
		size_t oldCol = col + (direction == EGD_LEFT);
		size_t newCol = col + (direction == EGD_RIGHT);
		if(!InsertColumn(newCol))
		{
			return false;
		}

		// Move window to new column
		HWND hwnd = m_Grid[oldCol][m_Grid.GetRowIndex()].m_Hwnd;
		m_Grid.InsertElement(newCol, 0, {hwnd, 1});
		m_Grid[oldCol].m_TotalHeightWeight -= m_Grid[oldCol][m_Grid.GetRowIndex()].m_HeightWeight;
		m_Grid.RemoveElement(oldCol, m_Grid.GetRowIndex());
		m_Grid[newCol].m_TotalHeightWeight = 1;

		// Ensure that the current window is the one we moved
		m_Grid.SetColumnIndex(newCol);
		m_Grid.SetRowIndex(0);

		Apply();
		return true;
	}

	size_t prevCol = m_Grid.GetColumnIndex();
	size_t prevRow = m_Grid.GetRowIndex();
	HWND hwnd = m_Grid[prevCol][0].m_Hwnd;
	bool bChanged = m_Grid.Move(direction, bWrapAround);
	if(bChanged)
	{
		if(direction & 0x1) // horizontal move
		{
			size_t currCol = m_Grid.GetColumnIndex();
			m_Grid.InsertElement(currCol, m_Grid.GetRowIndex(), {hwnd, 1});
			m_Grid[currCol].m_TotalHeightWeight += 1;

			// By getting here, we know the window is alone in the column,
			// so we must get rid of the column
			RemoveColumn(prevCol);

			// Ensure that the current window is the one we moved
			m_Grid.SetColumnIndex(currCol - (prevCol < currCol));
		}
		else // vertical move
		{
			// By bChanged being true, we know that there are multiple
			// windows in this column
			auto &col = m_Grid[prevCol];
			std::swap(col[prevRow], col[m_Grid.GetRowIndex()]);
		}

		Apply();
		return true;
	}

	return bChanged;
}

void WindowGrid::MoveTo(size_t column, size_t row)
{
	size_t prevColumn = m_Grid.GetColumnIndex();
	size_t prevRow = m_Grid.GetRowIndex();

	m_IsFullscreenMode = m_IsFullscreenMode && prevColumn == column && prevRow == row;

	m_Grid.SetColumnIndex(column);
	m_Grid.SetRowIndex(row);
}

void WindowGrid::MoveToEdgeFrom(EGridDirection direction)
{
	assert(EGD_UP <= direction && direction < EGD_COUNT);

	// Fullscreen workspaces should not change current window
	if(m_IsFullscreenMode){ return; }

	if(direction & 0x1) // horizontal move
	{
		m_Grid.SetColumnIndex((direction/2) * (m_Grid.ColumnCount()-1));

		// Ensure row index is still valid
		m_Grid.SetRowIndex(std::min(m_Grid.GetRowIndex(), m_Grid[m_Grid.GetColumnIndex()].size()));
	}

	// TODO create logic for vertical moves as well
}

void WindowGrid::Clear()
{
	m_Grid.Clear();
	m_TotalWidthWeight = 0;
}

bool WindowGrid::AttachToMonitor(MonitorInfo &monitor)
{
	if(m_IsActive)
	{
		return false;
	}

	m_MonitorRect = monitor.m_ScreenBounds;
	m_DpiScaling = monitor.m_DpiScaling;
	m_IsActive = true;
	return true;
}

bool WindowGrid::FocusWindow(HWND hwnd)
{
	// Move cursor into window
	RECT r;
	GetWindowRect(hwnd, &r);

	// Click on top corner of window
	//
	// This is a hack, but it should work in cases EXCEPT where there
	// are windows whose minimum width is greater than the width assigned
	// to it by the grid. If this is the case, then those windows may cover
	// up the top-left corners of windows to their right
	ClipCursor(0);
	SetCursorPos(r.left+10, r.top+4);

	INPUT click = {0};
	click.type = INPUT_MOUSE;
	click.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &click, sizeof(INPUT));

	click.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &click, sizeof(INPUT));

	// Clip cursor to within bounds of window
	SetCursorPos(r.left + (r.right-r.left)/2, r.top + (r.bottom-r.top)/2);
	if(s_ShouldClipCursor)
	{
		ClipCursor(&r);
	}

	return true;
}
