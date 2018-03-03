#include "w3_Grid.h"
#include <memory>
#include <unordered_set>
#include <queue>

#include <cassert>

_GridNode::_GridNode(_GridNode *defaultNeighbor/* =nullptr */, UINT8 startFlags /* = 0 */) :
	flags{startFlags}
{
	// pNeighbors initialization relies on this
	for(int i = 0; i < EGD_COUNT; ++i)
	{
		pNeighbors[i] = defaultNeighbor;
	}
}

_LinkedGrid::_LinkedGrid() :
	m_PrimaryDummy(&m_PrimaryDummy, _GridNode::EGF_HEAD | _GridNode::EGF_PRIMARY)
{}

_LinkedGrid::~_LinkedGrid(){}

void _LinkedGrid::Remove(_GridNode *pNode)
{
	_GridNode *pAbove = pNode->pNeighbors[EGD_UP];
	_GridNode *pBelow = pNode->pNeighbors[EGD_DOWN];

	pAbove->pNeighbors[EGD_DOWN] = pBelow;
	pBelow->pNeighbors[EGD_UP] = pAbove;

	// If only node in column
	if(pAbove == pBelow)
	{
		_GridNode *pLeftHead = pBelow->pNeighbors[EGD_LEFT];
		if(pLeftHead == &m_PrimaryDummy){ pLeftHead = pLeftHead->pNeighbors[EGD_LEFT]; }

		_GridNode *pRightHead = pBelow->pNeighbors[EGD_RIGHT];
		if(pRightHead == &m_PrimaryDummy){ pRightHead = pRightHead->pNeighbors[EGD_RIGHT]; }

		// Link nodes on left to nodes on right
		_GridNode *pLeftNext = pLeftHead->pNeighbors[EGD_DOWN];
		_GridNode *pRightNext = pRightHead->pNeighbors[EGD_DOWN];

		assert(pLeftHead->pNeighbors[EGD_DOWN] != pLeftHead);
		assert(pRightHead->pNeighbors[EGD_DOWN] != pRightHead);

		while(pLeftNext != pLeftHead && pRightNext != pRightHead)
		{
			pLeftNext->pNeighbors[EGD_RIGHT] = pRightNext;
			pRightNext->pNeighbors[EGD_LEFT] = pLeftNext;

			pLeftNext = pLeftNext->pNeighbors[EGD_DOWN];
			pRightNext = pRightNext->pNeighbors[EGD_DOWN];
		}

		while(pLeftNext != pLeftHead)
		{
			pLeftNext->pNeighbors[EGD_RIGHT] = pLeftNext->pNeighbors[EGD_UP]->pNeighbors[EGD_RIGHT];
			pLeftNext = pLeftNext->pNeighbors[EGD_DOWN];
		}
		while(pRightNext != pRightHead)
		{
			pRightNext->pNeighbors[EGD_LEFT] = pRightNext->pNeighbors[EGD_UP]->pNeighbors[EGD_LEFT];
			pRightNext = pRightNext->pNeighbors[EGD_DOWN];
		}

		// Remove column
		RemoveColumnHead(pBelow);
		delete pBelow;
	}
	else
	{
		// Find column head
		_GridNode *pHead = pBelow;
		while(!pHead->IsHead())
		{
			pHead = pHead->pNeighbors[EGD_DOWN];
		}

		if(pBelow == pHead){ pBelow = pHead->pNeighbors[EGD_UP]; }

		// Set left/right neighbors pointing to pNode to point to pBelow
		_GridNode *pLeftHead = pHead->pNeighbors[EGD_LEFT];
		_GridNode *pTemp = pLeftHead->pNeighbors[EGD_DOWN];
		while(pTemp != pLeftHead)
		{
			if(pTemp->pNeighbors[EGD_RIGHT] == pNode)
			{
				pTemp->pNeighbors[EGD_RIGHT] = pBelow;
			}
		}

		_GridNode *pRightHead = pHead->pNeighbors[EGD_RIGHT];
		pTemp = pRightHead->pNeighbors[EGD_DOWN];
		while(pTemp != pRightHead)
		{
			if(pTemp->pNeighbors[EGD_LEFT] == pNode)
			{
				pTemp->pNeighbors[EGD_LEFT] = pBelow;
			}
		}
	}
}

void _LinkedGrid::InsertColumnHead(_GridNode *pNewHead, _GridNode *pLeftHead)
{
	// Ensure heads are actually dummy nodes
	assert(pNewHead->IsHead());
	assert(pLeftHead->IsHead());

	// Insert node to the right
	_GridNode *pPrevRight = pLeftHead->pNeighbors[EGD_RIGHT];
	pNewHead->pNeighbors[EGD_LEFT] = pLeftHead;
	pLeftHead->pNeighbors[EGD_RIGHT] = pNewHead;

	pNewHead->pNeighbors[EGD_RIGHT] = pPrevRight;
	pPrevRight->pNeighbors[EGD_LEFT] = pNewHead;
}

void _LinkedGrid::RemoveColumnHead(_GridNode *pHead)
{
	// Ensure pHead points to an empty node in the dummy list
	assert(pHead->IsHead());
	assert(pHead->pNeighbors[EGD_UP] == pHead && pHead->pNeighbors[EGD_DOWN] == pHead);

	_GridNode *pLeft = pHead->pNeighbors[EGD_LEFT], *pRight = pHead->pNeighbors[EGD_RIGHT];
	pLeft->pNeighbors[EGD_RIGHT] = pRight;
	pRight->pNeighbors[EGD_LEFT] = pLeft;
}

void _LinkedGrid::PushTop(_GridNode *pNode, _GridNode *pColumnHead)
{
	assert(pColumnHead->IsHead());

	// Insert below pColumnHead
	_GridNode *pLowerNeighbor = pColumnHead->pNeighbors[EGD_DOWN];

	pNode->pNeighbors[EGD_UP] = pColumnHead;
	pColumnHead->pNeighbors[EGD_DOWN] = pNode;

	pNode->pNeighbors[EGD_DOWN] = pLowerNeighbor;
	pLowerNeighbor->pNeighbors[EGD_UP] = pNode;

	// Get left/right column heads
	_GridNode *pLeftHead = pColumnHead->pNeighbors[EGD_LEFT];
	if(pLeftHead == &m_PrimaryDummy){ pLeftHead = pLeftHead->pNeighbors[EGD_LEFT]; }

	_GridNode *pRightHead = pColumnHead->pNeighbors[EGD_RIGHT];
	if(pRightHead == &m_PrimaryDummy){ pRightHead = pRightHead->pNeighbors[EGD_RIGHT]; }

	_GridNode *pLeftNeighbor = nullptr, *pRightNeighbor = nullptr;

	// If pNode is the only node in column, make all left/right neighbors point to it
	if(pLowerNeighbor == pColumnHead)
	{
		_GridNode *pTemp = pLeftHead->pNeighbors[EGD_UP];
		while(pTemp != pLeftHead)
		{
			pTemp->pNeighbors[EGD_RIGHT] = pNode;
			pLeftNeighbor = pTemp;
			pTemp = pTemp->pNeighbors[EGD_UP];
		}

		pTemp = pRightHead->pNeighbors[EGD_UP];
		while(pTemp != pRightHead)
		{
			pTemp->pNeighbors[EGD_LEFT] = pNode;
			pRightNeighbor = pTemp;
			pTemp = pTemp->pNeighbors[EGD_UP];
		}
	}
	else // Otherwise, only set the first nodes of the adjacent columns to point to pNode
	{
		pLeftNeighbor = pLeftHead->pNeighbors[EGD_DOWN];
		pLeftNeighbor->pNeighbors[EGD_RIGHT] = pNode;

		pRightNeighbor = pRightHead->pNeighbors[EGD_DOWN];
		pRightNeighbor->pNeighbors[EGD_LEFT] = pNode;
	}

	// Set left/right neighbors
	pNode->pNeighbors[EGD_LEFT] = pLeftNeighbor;
	pNode->pNeighbors[EGD_RIGHT] = pRightNeighbor;
}

void _LinkedGrid::Clear()
{
	_GridNode *pCurrentDummy = m_PrimaryDummy.pNeighbors[EGD_RIGHT];

	// Iterate through each column
	while(pCurrentDummy != &m_PrimaryDummy)
	{
		// Delete all nodes in current column
		_GridNode *pCurrentNode  = pCurrentDummy->pNeighbors[EGD_DOWN];
		_GridNode *pNextNode;
		while(pCurrentNode != pCurrentDummy)
		{
			pNextNode = pCurrentNode->pNeighbors[EGD_DOWN];
			delete pCurrentNode;
			pCurrentNode = pNextNode;
		}

		_GridNode *pNextDummy = pCurrentDummy->pNeighbors[EGD_RIGHT];
		delete pCurrentDummy;
		pCurrentDummy = pNextDummy;
	}

	static_assert(EGD_COUNT == 4, "");
	m_PrimaryDummy.pNeighbors[EGD_UP] = m_PrimaryDummy.pNeighbors[EGD_RIGHT] = \
	m_PrimaryDummy.pNeighbors[EGD_DOWN] = m_PrimaryDummy.pNeighbors[EGD_LEFT] = &m_PrimaryDummy;
}
