#include "w3_Monitor.h"
#include <cassert>
#include <memory>

MonitorGrid::MonitorGrid() :
	m_pCurrentNode(nullptr)
{}

bool MonitorGrid::Insert(MonitorInfo &&info)
{
	// NOTE: This entire function is a test implementation. It may be rewritten from scratch
	// in the future.
	// @TODO - add logic for vertical configurations (requires _LinkedGrid::PushBelow)
	GridNode<MonitorInfo> *pNode = new (std::nothrow) GridNode<MonitorInfo>(info);
	if(!pNode){ return false; }

	GridNode<MonitorInfo> *pHead = new (std::nothrow) GridNode<MonitorInfo>(info);
	if(!pHead){ delete pNode; return false; }
	pHead->MakeHead();

	// Insert new column with node at appropriate location
	_GridNode *pPrev = &m_PrimaryDummy;
	_GridNode *pCurr = m_PrimaryDummy.pNeighbors[EGD_RIGHT];
	while(!pCurr->IsPrimary() &&
		((GridNode<MonitorInfo>*)pCurr)->data.screenBounds.left < info.screenBounds.left)
	{
		pPrev = pCurr;
		pCurr = pCurr->pNeighbors[EGD_RIGHT];
	}

	InsertColumnHead(pHead, pPrev);
	PushTop(pNode, pHead);

	m_pCurrentNode = !m_pCurrentNode ? pNode : m_pCurrentNode;
	return true;
}

const MonitorInfo &MonitorGrid::GetCurrentMonitor() const
{
	assert(m_pCurrentNode);
	return m_pCurrentNode->data;
}

const MonitorInfo &MonitorGrid::Move(EGridDirection direction)
{
	assert(direction >= 0);
	assert(direction < EGD_COUNT);
	assert(m_pCurrentNode);

	do
	{
		m_pCurrentNode = (GridNode<MonitorInfo>*)m_pCurrentNode->pNeighbors[direction];
	} while(m_pCurrentNode->IsHead());

	return GetCurrentMonitor();
}

void MonitorGrid::Clear()
{
	_LinkedGrid::Clear();

	m_pCurrentNode = nullptr;
}