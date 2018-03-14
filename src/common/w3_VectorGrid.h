#pragma once

#include <vector>
#include "w3_Core.h"

template<typename T>
struct GridHead
{
	std::vector<T> m_Elems;

	inline T &operator[](size_t n){ return m_Elems[n]; }
	inline const T &operator[](size_t n) const{ return m_Elems[n]; }
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
	VectorGrid() : m_ColumnIndex(0), m_RowIndex(0) {}

	VectorGrid(const VectorGrid &other) = default;
	VectorGrid(VectorGrid &&other);

	VectorGrid &operator=(const VectorGrid &other) = default;
	VectorGrid &operator=(VectorGrid &&other);

	/**
	 * @brief Removes all nodes and columns from the grid
	 */
	void Clear();

	/**
	 * @brief Insert a column before the column at position pos
	 *
	 * Perfect forwards the arguments after pos to the constructor of HeadType.
	 * Therefore this method can be used even with custom HeadTypes.
	 *
	 * @return true if pos is valid an insertion is successful, else false
	 */
	template<typename... Ts>
	bool InsertColumn(size_t pos, Ts&&... ts);

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

	/**
	 * @brief Return the nth column
	 */
	inline HeadType &operator[](size_t n){ return m_Columns[n]; }
	inline const HeadType &operator[](size_t n) const{ return m_Columns[n]; }

	/**
	 * @brief Return current element, if it exists
	 */
	NodeType *GetCurrent(){ return _GetCurrent(); }
	inline const NodeType *GetCurrent() const{ return _GetCurrent(); }

	/**
	 * @brief Move in a particular direction
	 *
	 * @return True if the current position actually changed
	 */
	bool Move(EGridDirection direction, bool bWrapAround=true);

	/**
	 * @brief Return the number of columns in the grid
	 */
	inline size_t ColumnCount() const{ return m_Columns.size(); }

protected:
	/**
	 * @brief Get row index of adjacent column
	 *
	 * This method can be overridden by child classes to change the behavior of
	 * Move(). In particular, child classes can use this method as a chance to store
	 * information about the current row index in the head/data nodes.
	 *
	 * @note Neither m_RowIndex nor m_ColumnIndex have been modified by Move() yet
	 *       when this is called.
	 */
	virtual size_t AdjacentRowIndex(EGridDirection dir, size_t newColumnIndex);

private:
	NodeType *_GetCurrent() const;

protected:
	std::vector<HeadType> m_Columns;

	size_t m_ColumnIndex;
	size_t m_RowIndex;
};

#include "w3_VectorGrid.inl"
