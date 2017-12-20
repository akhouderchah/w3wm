#include "w3_Monitor.h"
#include <memory>
#include <unordered_set>
#include <queue>

MonitorQuadTree::MonitorQuadTree() :
	m_pPrimaryNode(nullptr)
{}

MonitorQuadTree::~MonitorQuadTree()
{
	Clear();
}

bool MonitorQuadTree::Initialize(MonitorInfo &info)
{
	if(m_pPrimaryNode)
	{
		return false;
	}

	m_pPrimaryNode = new (std::nothrow) QuadNode<MonitorInfo>{ {}, info };
	if(!m_pPrimaryNode)
	{
		return false;
	}

	for(int i = 0; i < EQD_COUNT; ++i)
	{
		m_pPrimaryNode->pNodes[i] = m_pPrimaryNode;
	}

	return true;
}

bool MonitorQuadTree::Insert(MonitorInfo &info)
{
	// @TODO - Add vertical logic also

	QuadNode<MonitorInfo> *pTestNode = nullptr, *pCurrNode = m_pPrimaryNode;

	QuadNode<MonitorInfo> *pNewNode = new (std::nothrow) QuadNode<MonitorInfo>{ {}, info };
	if(!pNewNode)
	{
		return false;
	}

	while(1)
	{
		if(info.screenBounds.left > pCurrNode->data.screenBounds.left)
		{
			pTestNode = pCurrNode->pNodes[EQD_RIGHT];
			if(info.screenBounds.left < pTestNode->data.screenBounds.left || pCurrNode == pTestNode)
			{
				// Insert between current and test nodes
				pCurrNode->pNodes[EQD_RIGHT] = pNewNode;
				pTestNode->pNodes[EQD_LEFT] = pNewNode;

				pNewNode->pNodes[EQD_RIGHT] = pTestNode;
				pNewNode->pNodes[EQD_LEFT] = pCurrNode;
				pNewNode->pNodes[EQD_UP] = pNewNode;
				pNewNode->pNodes[EQD_DOWN] = pNewNode;

				break;
			}
		}
		else
		{
			pTestNode = pCurrNode->pNodes[EQD_LEFT];
			if(info.screenBounds.left > pTestNode->data.screenBounds.left || pCurrNode == pTestNode)
			{
				// Insert between current and test nodes
				pCurrNode->pNodes[EQD_LEFT] = pNewNode;
				pTestNode->pNodes[EQD_RIGHT] = pNewNode;

				pNewNode->pNodes[EQD_LEFT] = pTestNode;
				pNewNode->pNodes[EQD_RIGHT] = pCurrNode;
				pNewNode->pNodes[EQD_UP] = pNewNode;
				pNewNode->pNodes[EQD_DOWN] = pNewNode;

				break;
			}
		}

		pCurrNode = pTestNode;
	}

	return true;
}

void MonitorQuadTree::Clear()
{
	std::unordered_set<QuadNode<MonitorInfo>*> pNodes;
	pNodes.insert(m_pPrimaryNode);

	std::queue<QuadNode<MonitorInfo>*> pUnexpandedNodes;
	pUnexpandedNodes.push(m_pPrimaryNode);

	while(!pUnexpandedNodes.empty())
	{
		QuadNode<MonitorInfo> *pCurrNode = pUnexpandedNodes.front();
		pUnexpandedNodes.pop();

		for(int i = 0; i < EQD_COUNT; ++i)
		{
			if(pNodes.insert(pCurrNode->pNodes[i]).second)
			{
				pUnexpandedNodes.push(pCurrNode->pNodes[i]);
			}
		}
	}

	for(QuadNode<MonitorInfo> *pNode : pNodes)
	{
		delete pNode;
	}

	m_pPrimaryNode = nullptr;
}
