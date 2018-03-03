#include "w3_Window.h"
#include <memory>

WindowGrid::WindowGrid()
{
	m_pCurrentNode = &m_PrimaryDummy;
}

// TODO THIS IS A TEST IMPLEMENTATION
bool WindowGrid::Insert(HWND hwnd)
{
	//GridNode<NodeInfo> *pNode = new (std::nothrow) GridNode<NodeInfo>();
	return false;
}

void WindowGrid::Apply()
{

}

void WindowGrid::Clear()
{
	_LinkedGrid::Clear();

	m_pCurrentNode = &m_PrimaryDummy;
}
