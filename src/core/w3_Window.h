#pragma once

#include "w3_Grid.h"

/**
 * @brief Structure representing the position of windows in a single monitor
 *
 * Much of the window tiling functionality is contained in this class.
 *
 * @note This class will usually not be used directly, but will instead be
 *       be used by MonitorGrid or WindowManager.
 */
class WindowGrid
{
public:
	WindowGrid(LPRECT pMonitorRect, float dpiScaling=1.f);
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
};
