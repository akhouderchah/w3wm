#pragma once

#include <windows.h>

enum EQuadDirection
{
	EQD_UP,
	EQD_RIGHT,
	EQD_DOWN,
	EQD_LEFT,
	EQD_COUNT
};

template <typename T>
struct QuadNode
{
	QuadNode<T> *pNodes[EQD_COUNT];
	T data;
};

struct MonitorInfo
{
	HDC hdcMonitor;
	RECT screenBounds;
};

class MonitorQuadTree
{
public:
	MonitorQuadTree();
	~MonitorQuadTree();

	bool Initialize(MonitorInfo &info);

	bool Insert(MonitorInfo &info);
	void Clear();

	QuadNode<MonitorInfo> *GetPrimaryMonitor() const{ return m_pPrimaryNode; }
private:
	QuadNode<MonitorInfo> *m_pPrimaryNode;
};
