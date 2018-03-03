#include <cassert>

template<typename NodeType, typename HeadType>
void VectorGrid<NodeType, HeadType>::Clear()
{
}

template<typename NodeType, typename HeadType>
bool VectorGrid<NodeType, HeadType>::InsertColumn(size_t pos, HeadType &&head)
{
	assert(col <= m_Columns.size());

	try
	{
		m_Columns.insert(m_Columns.cbegin() + pos, std::move(head));
	}
	catch(std::bad_alloc &ba)
	{
		return false;
	}

	return true;
}

template<typename NodeType, typename HeadType>
bool VectorGrid<NodeType, HeadType>::InsertColumn(size_t pos, const HeadType &head)
{
	assert(col <= m_Columns.size());

	try
	{
		m_Columns.insert(m_Columns.cbegin() + pos, head);
	}
	catch(std::bad_alloc &ba)
	{
		return false;
	}

	return true;
}

template<typename NodeType, typename HeadType>
void VectorGrid<NodeType, HeadType>::RemoveColumn(size_t pos)
{
	assert(col < m_Columns.size());

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
	catch(std::bad_alloc &ba)
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
	catch(std::bad_alloc &ba)
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
