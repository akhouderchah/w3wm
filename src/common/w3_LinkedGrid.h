#pragma once

#include <windows.h>

enum EGridDirection
{
	EGD_UP,
	EGD_RIGHT,
	EGD_DOWN,
	EGD_LEFT,
	EGD_COUNT
};

/**
 * @brief Base struct for templated GridNodes
 *
 * @note Users should mostly use a templated GridNode struct. This struct
 *       exists so that _LinkedGrid can have type-agnostic functionality.
 */
struct _GridNode
{
	_GridNode(_GridNode *defaultNeighbor=nullptr, UINT8 startFlags=0);

	inline bool IsHead() const{ return flags & (UINT8)EGF_HEAD; }
	inline bool IsPrimary() const{ return !!(flags & (UINT8)EGF_PRIMARY); }

	_GridNode *pNeighbors[EGD_COUNT];
	UINT8 flags;

	enum EGridFlags
	{
		EGF_HEAD = 0x1,
		EGF_PRIMARY = 0x2,
	};
};

/**
 * @brief Base linked grid class
 *
 * Contains the majority of the linked grid functionality. Derived classes can choose
 * to consist of only one child type of _GridNode, and may provide very different interfaces
 * to users.
 *
 * This data structure is a variant of a quadtree, and is used to represent the relative locations of
 * monitors and windows in the main application. It consists of a set of columns, each of which is
 * similar to a circularly linked list with a dummy node. The dummy nodes of each column is at the
 * "top" of the column, and these dummy nodes are connected to each other to form a "dummy list",
 * which is a circularly linked list of column heads. In addition, all nodes have left and right
 * neighbors, which roughly forms a set of circularly linked rows (i.e. starting from any node and
 * moving left/right, one will eventually have a cycle. The starting node may not be a part of
 * this cycle, however).
 */
class _LinkedGrid
{
public:
	_LinkedGrid();
	virtual ~_LinkedGrid() = 0;

	/**
	 * @brief Detaches and deallocates all nodes besides m_PrimaryDummy
	 */
	void Clear();

protected:
	/**
	 * @brief Removes pNode from the grid
	 *
	 * @note pNode is NOT deallocated by this call. However, if pNode is the
	 *       only node in its column, the column head WILL be deallocated by
	 *       this call.
	 */
	void Remove(_GridNode *pNode);

	/**
	 * @brief Inserts pNewHead to the right of pLeftHead
	 */
	void InsertColumnHead(_GridNode *pNewHead, _GridNode *pLeftHead);

	/**
	 * @brief Removes an empty column head from the dummy list
	 *
	 * @note pHead is NOT deallocated by this call
	 */
	void RemoveColumnHead(_GridNode *pHead);

	/**
	 * @brief Inserts pNode below pColumnHead
	 */
	void PushTop(_GridNode *pNode, _GridNode *pColumnHead);

protected:
	_GridNode m_PrimaryDummy;
};

template <typename T>
struct GridNode : public _GridNode
{
	GridNode(const T &d, UINT8 startFlags=0, _GridNode *defaultNeighbor=nullptr) :
		_GridNode(defaultNeighbor, startFlags), data(d){}

	inline void MakeHead(){ flags |= EGF_HEAD; for(int i = 0; i < EGD_COUNT; ++i)pNeighbors[i] = this; }

	T data;
};
