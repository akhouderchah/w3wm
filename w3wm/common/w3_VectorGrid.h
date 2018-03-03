#pragma once

#include <vector>

template<typename T>
struct GridHead
{
	std::vector<T> m_Elems;

	inline T& operator[](size_t n){ return m_Elems[n]; }
	inline const T& operator[](size_t n) const{ return m_Elems[n]; }
};

/**
 * @brief Grid class represented as a vector of vectors
 *
 * More concretely, this class is a vector of HeadTypes. The HeadType
 * is required to have a vector-like class called m_Elems, and to offer
 * at least a const operator[] method. Any class/struct satisfying these
 * attributes may be used as a HeadType. In the usual case, this class
 * is essentially just a vector<vector<NodeType>>.
 */
template<typename NodeType, typename HeadType = GridHead<NodeType>>
class VectorGrid
{
public:
	/**
	 * @brief Removes all nodes and columns from the grid
	 */
	void Clear();

	/**
	 * @brief Insert a column before the column at position pos
	 *
	 * @return true if pos is valid an insertion is successful, else false
	 */
	bool InsertColumn(size_t pos, HeadType &&elem);
	bool InsertColumn(size_t pos, const HeadType &elem);

	/**
	 * @brief Remove column at position pos, if there exists one
	 *
	 * @note This operation causes all of the columns after position pos
	 *       to shift to the left, and therefore will not be particularly
	 *       efficient when there are many columns after position pos.
	 */
	void RemoveColumn(size_t pos);

	/**
	 * @brief Insert an element at the indicated row and column.
	 *
	 * @note This method does NOT create columns automatically. If the specified
	 *       column does not already exist, the method will fail.
	 * @note This method will not fill in empty rows. If the specified row is greater
	 *       than 1 + the current column size, the method will fail.
	 *
	 * @return True if (col, row) is valid and the insertion succeeded.
	 */
	bool InsertElement(size_t col, size_t row, NodeType &&elem);
	bool InsertElement(size_t col, size_t row, const NodeType &elem);

	/**
	 * @brief Remove an element at the indicated row and column.
	 *
	 * @note This method assumes that both col and row are valid.
	 */
	void RemoveElement(size_t col, size_t row);

	inline HeadType &operator[](size_t n){ return m_Columns[n]; }
	inline const HeadType &operator[](size_t n) const{ return m_Columns[n]; }

protected:
	std::vector<HeadType> m_Columns;
};

#include "w3_VectorGrid.inl"
