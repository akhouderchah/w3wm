#pragma once

#include "w3_Grid.h"

struct WindowInfo
{

};

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
