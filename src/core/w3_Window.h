#pragma once

#include "w3_Grid.h"

/**
 * @brief Structure representing the position of windows in a single monitor
 *
 * Much of the window tiling functionality is contained in this class.
 *
 * @note This class will usually not be used directly, but will instead be
 *       be used by MonitorGrid.
 */
class WindowGrid
{
public:
	~WindowGrid(){ Clear(); }

	/**
	 * @brief Add an untracked window, placing it in a new, right-most column
	 */
	bool Insert(HWND hwnd);

	/**
	 * @brief Move/resize windows to match the grid representation
	 */
	void Apply();

	/**
	 * @brief Untrack all windows
	 */
	void Clear();
private:
	struct Node
	{
		HWND m_Hwnd;
		LONG m_TopBound;
		LONG m_BottomBound;
	};

	struct Column : GridHead<Node>
	{
		Column(LONG leftBound, LONG rightBound) : m_LeftBound(leftBound), m_RightBound(rightBound){}
		LONG m_LeftBound;
		LONG m_RightBound;
	};

	VectorGrid<Node, Column> m_Grid;
};

/*
class WindowGrid : public _LinkedGrid
{
public:
	WindowGrid();
	~WindowGrid(){ Clear(); }

	bool Insert(HWND hwnd);
	bool Remove(const WindowInfo &wndInfo);

	void Apply();

	void Clear();

private:
	struct ColumnInfo
	{
		LONG left;
		LONG right;
		UINT16 count;
		UINT16 columnFlags;
	};

	struct NodeInfo
	{
		HWND hwnd;
		LONG top;
		LONG bottom;
	};

	_GridNode *m_pCurrentNode;
};
*/
