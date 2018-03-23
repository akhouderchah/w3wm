#pragma once

#include <windows.h>
#include "w3_Grid.h"

class MonitorGrid
{
public:
	MonitorGrid();
	~MonitorGrid(){ Clear(); }

	/**
	 * @brief Add new, untracked monitor to the grid
	 * @return If successful, pointer to the inserted info, else nullptr.
	 */
	MonitorInfo *Insert(MonitorInfo &&info);

	/**
	 * @brief Set the current monitor to the nearest one in direction
	 * @return True if the current position actually changed
	 */
	bool Move(EGridDirection direction, bool bWrapAround=true);

	/**
	 * @brief Get the workspace index attached to the current monitor
	 * @return Workspace index if there is one, -1 (max size_t value) else
	 */
	size_t GetWorkspaceIndex();

	/**
	 * @brief Move to the monitor with the attached workspace
	 * @return True if such a monitor was found, false otherwise
	 */
	bool MoveToWorkspace(size_t workspaceIndex);

	/**
	 * @brief Untrack all monitors
	 */
	void Clear();

	/**
	 * @return Whether or not this grid contains the primary monitor
	 */
	inline bool HasPrimary() const{ return m_HasPrimary; }

private:
	VectorGrid<MonitorInfo> m_Grid;
	bool m_HasPrimary;
};
