#include <cassert>
#include <algorithm>
#undef min

template<typename NodeType, typename HeadType>
VectorGrid<NodeType, HeadType>::VectorGrid(VectorGrid &&other) :
	m_Columns(std::move(other.m_Columns)),
	m_ColumnIndex(other.m_ColumnIndex),
	m_RowIndex(other.m_RowIndex)
{}

template<typename NodeType, typename HeadType>
VectorGrid<NodeType, HeadType> &VectorGrid<NodeType, HeadType>::operator=(VectorGrid &&other)
{
	Clear();
	m_Columns = std::move(other.m_Columns);
	m_ColumnIndex = other.m_ColumnIndex;
	m_RowIndex = other.m_RowIndex;

	return *this;
}

template<typename NodeType, typename HeadType>
void VectorGrid<NodeType, HeadType>::Clear()
{
	m_Columns.clear();
}

template<typename NodeType, typename HeadType> template<typename... Ts>
bool VectorGrid<NodeType, HeadType>::InsertColumn(size_t pos, Ts&&... ts)
{
	assert(pos <= m_Columns.size());

	try
	{
		m_Columns.emplace(m_Columns.cbegin() + pos, std::forward<Ts>(ts)...);
	}
	catch(std::bad_alloc &)
	{
		return false;
	}

	return true;
}

template<typename NodeType, typename HeadType>
void VectorGrid<NodeType, HeadType>::RemoveColumn(size_t pos)
{
	assert(pos < m_Columns.size());

	m_Columns.erase(m_Columns.begin() + pos);
}

template<typename NodeType, typename HeadType>
bool VectorGrid<NodeType, HeadType>::InsertElement(size_t col, size_t row, NodeType &&elem)
{
	assert(col <= m_Columns.size());
	assert(row <= m_Columns[col].m_Elems.size());

	auto& colElems = m_Columns[col].m_Elems;
	try
	{
		colElems.insert(colElems.cbegin() + row, std::move(elem));
	}
	catch(std::bad_alloc &)
	{
		return false;
	}

	return true;
}

template<typename NodeType, typename HeadType>
bool VectorGrid<NodeType, HeadType>::InsertElement(size_t col, size_t row, const NodeType &elem)
{
	assert(col <= m_Columns.size());
	assert(row <= m_Columns[col].m_Elems.size());

	auto& colElems = m_Columns[col].m_Elems;
	try
	{
		colElems.insert(colElems.cbegin() + row, elem);
	}
	catch(std::bad_alloc &)
	{
		return false;
	}

	return true;
}

template<typename NodeType, typename HeadType>
void VectorGrid<NodeType, HeadType>::RemoveElement(size_t col, size_t row)
{
	assert(col < m_Columns.size());
	assert(row < m_Columns[col].m_Elems.size());

	auto &colElems = m_Columns[col].m_Elems;
	colElems.erase(colElems.begin() + row);
}

template<typename NodeType, typename HeadType>
NodeType *VectorGrid<NodeType, HeadType>::_GetCurrent() const
{
	if(m_ColumnIndex >= ColumnCount())
	{
		return nullptr;
	}

	auto& col = m_Columns[m_ColumnIndex];
	if(m_RowIndex >= col.m_Elems.size())
	{
		return nullptr;
	}

	return const_cast<NodeType*>(&col[m_RowIndex]);
}

template<typename NodeType, typename HeadType>
bool VectorGrid<NodeType, HeadType>::Move(EGridDirection direction, bool bWrapAround)
{
	assert(0 <= direction && direction < EGD_COUNT);

	size_t newIndex, currentIndex;

	if(direction % 2 == 0)	// moving vertically
	{
		int moveAmt = direction - 1;
		newIndex = m_RowIndex + moveAmt;
		currentIndex = m_RowIndex;

		if(ColumnCount() <= m_ColumnIndex)
		{
			return false;
		}

		auto& col = m_Columns[m_ColumnIndex];

		size_t rowCount = col.m_Elems.size();
		if(rowCount <= newIndex)
		{
			if(bWrapAround && rowCount)
			{
				newIndex = int(!direction) * (rowCount - 1);
			}
			else
			{
				return false;
			}
		}

		m_RowIndex = newIndex;
	}
	else					// moving horizontally
	{
		int moveAmt = 2 - direction;
		newIndex = m_ColumnIndex + moveAmt;
		currentIndex = m_ColumnIndex;

		size_t colCount = ColumnCount();
		if(colCount <= newIndex)
		{
			if(bWrapAround && colCount)
			{
				newIndex = (direction-1)/2 * (colCount - 1);
			}
			else
			{
				return false;
			}
		}

		m_RowIndex = AdjacentRowIndex(direction, newIndex);
		m_ColumnIndex = newIndex;
	}

	return (newIndex != currentIndex);
}

template<typename NodeType, typename HeadType>
size_t VectorGrid<NodeType, HeadType>::AdjacentRowIndex(EGridDirection dir, size_t newColumnIndex)
{
	assert(newColumnIndex < ColumnCount());
	return std::min(m_RowIndex, m_Columns[newColumnIndex].m_Elems.size()-1);
}
