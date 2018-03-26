#pragma once

#include "w3_Grid.h"

/**
 * @brief Structure representing the position of windows in a single monitor
 *
 * Much of the window tiling functionality is contained in this class.
 *
 * @note This class will usually not be used directly, but will instead be
 *       be used by w3Context.
 */
class WindowGrid
{
public:
	WindowGrid();
	~WindowGrid(){ Clear(); }

	WindowGrid(WindowGrid &&other);
	WindowGrid &operator=(WindowGrid &&other);

	/**
	 * @brief Add an untracked window, placing it in a new, right-most column
	 */
	bool Insert(HWND hwnd, EGridDirection directionFrom=EGD_LEFT);

	/**
	 * @brief Remove an element from the grid
	 */
	void Remove(size_t col, size_t row);

	/**
	 * @brief Remove the current element from the grid
	 */
	inline void RemoveCurrent(){ Remove(m_Grid.GetColumnIndex(), m_Grid.GetRowIndex()); }

	/**
	 * @brief Get the current window in this grid
	 * @return 0 if grid is empty. Else the proper HWND.
	 */
	HWND GetCurrent();

	/**
	 * @brief Find the location of an HWND in the grid
	 * @return True if the HWND is in this grid
	 */
	bool Find(HWND wnd, size_t *pCol, size_t *pRow);

	/**
	 * @brief Move/resize windows to match the grid representation
	 */
	void Apply();

	/**
	 * @brief Set focus to the window in the specified direction
	 * @return True if the current position actually changed
	 */
	bool MoveFocus(EGridDirection direction, bool bWrapAround=true);

	/**
	 * @brief Move the current window in the specified direction
	 * @return True if the window position has actually changed
	 */
	bool MoveWindow(EGridDirection direction, bool bWrapAround=true);

	/**
	 * @brief Set current position to the specified column and row
	 * @note This function does NOT verify that the column and row indices
	 *       are valid.
	 */
	void MoveTo(size_t column, size_t row);

	/**
	 * @brief Set current position to grid edge, coming from specified direction
	 */
	void MoveToEdgeFrom(EGridDirection direction);

	/**
	 * @brief Untrack all windows
	 */
	void Clear();

	/**
	 * @brief Activate and set the virtual screen coords of this grid
	 * @note No changes will be visible until the next Apply()
	 * @return Whether or not attachment was successful
	 */
	bool AttachToMonitor(MonitorInfo &monitor);

	/**
	 * @brief Set focus on the current window
	 */
	inline bool FocusCurrent()
	{
		Node *pNode = m_Grid.GetCurrent();
		if(pNode) return FocusWindow(pNode->m_Hwnd); return false;
	}

	/**
	 * @brief Toggle whether or not the current window is fullscreened
	 */
	void ToggleFullscreen(){ m_IsFullscreenMode = !m_IsFullscreenMode && m_Grid.ColumnCount(); }

	/**
	 * @brief Return whether or not this grid has any nodes
	 */
	inline bool IsEmpty() const{ return !m_Grid.ColumnCount(); }

	inline static void ShouldClipCursor(bool shouldClip){ s_ShouldClipCursor = shouldClip; }
private:
	/**
	 * @brief Set focus to the specified window
	 */
	bool FocusWindow(HWND hwnd);

	bool InsertColumn(size_t colPos);
	void RemoveColumn(size_t colPos);

private:
	struct Node
	{
		HWND m_Hwnd;
		UINT32 m_HeightWeight;
	};

	struct Column : GridHead<Node>
	{
		Column(UINT32 widthWeight) : m_WidthWeight(widthWeight),
									 m_TotalHeightWeight(0){}
		UINT32 m_WidthWeight;
		UINT32 m_TotalHeightWeight;
	};

	VectorGrid<Node, Column> m_Grid;

	UINT32 m_DefaultWidthWeight;
	UINT32 m_TotalWidthWeight;

	RECT m_MonitorRect;
	float m_DpiScaling;
	bool m_IsActive;
	bool m_IsFullscreenMode; // Is the current window fullscreened?

	static bool s_ShouldClipCursor;
};
