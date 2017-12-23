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
	~MonitorGrid(){ Clear(); }

	bool Insert(MonitorInfo &info);
};
