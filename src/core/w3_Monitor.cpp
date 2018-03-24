#include "w3_Monitor.h"
#include <cassert>

MonitorGrid::MonitorGrid() :
	m_HasPrimary(false)
{
}

MonitorInfo *MonitorGrid::Insert(MonitorInfo &&info)
{
	// TODO This is an incomplete implementation that only works for
	// horizontal monitor setups

	// Find column position to insert
	size_t col = 0;
	for(col; col < m_Grid.ColumnCount(); ++col)
	{
		auto &curr = m_Grid[col];
		if(curr.size() == 0) continue;

		if(info.m_ScreenBounds.left < curr[0].m_ScreenBounds.left) break;
	}

	// Create column and insert info into grid
	if(m_Grid.InsertColumn(col))
	{
		if(m_Grid.InsertElement(col, 0, std::move(info)))
		{
			if(m_Grid[col][0].m_ScreenBounds.left == 0 && m_Grid[col][0].m_ScreenBounds.top == 0)
			{
				m_HasPrimary = true;
			}

			return &m_Grid[col][0];
		}
		else
		{
			m_Grid.RemoveColumn(col);
		}
	}

	return nullptr;
}

bool MonitorGrid::Move(EGridDirection direction, bool bWrapAround)
{
	return m_Grid.Move(direction, bWrapAround);
}

size_t MonitorGrid::GetWorkspaceIndex()
{
	MonitorInfo *pMon = m_Grid.GetCurrent();
	if(pMon)
	{
		return pMon->m_WorkspaceIndex;
	}
	return -1l;
}

bool MonitorGrid::MoveToWorkspace(size_t workspaceIndex)
{
	// Just search the entire grid for now. Not expecting users
	// to have so many monitors that this will be slow
	for(size_t col = 0; col < m_Grid.ColumnCount(); ++col)
	{
		auto &currCol = m_Grid[col];
		for(size_t row = 0; row < currCol.size(); ++row)
		{
			if(currCol[row].m_WorkspaceIndex == workspaceIndex)
			{
				m_Grid.SetColumnIndex(col);
				m_Grid.SetRowIndex(row);
				return true;
			}
		}
	}

	return false;
}

void MonitorGrid::Clear()
{
	m_Grid.Clear();
	m_HasPrimary = false;
}
