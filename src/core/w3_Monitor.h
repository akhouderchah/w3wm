#pragma once

#include <windows.h>
#include "w3_Grid.h"

struct MonitorInfo
{
	HDC hMonitor;
	RECT screenBounds;
};

class MonitorGrid : public _LinkedGrid
{
public:
	MonitorGrid();
	~MonitorGrid(){ Clear(); }

	bool Insert(MonitorInfo &&info);

	const MonitorInfo &GetCurrentMonitor() const;
	const MonitorInfo &Move(EGridDirection direction);

	inline bool IsEmpty() const{ return m_pCurrentNode == nullptr; }

	void Clear();

private:
	GridNode<MonitorInfo> *m_pCurrentNode;
};
