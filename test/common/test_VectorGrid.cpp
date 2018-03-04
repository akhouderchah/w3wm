#include "common/w3_VectorGrid.h"

#include "gtest/gtest.h"

namespace w3 {

template <typename T>
class VectorGridTest : public ::testing::Test
{
public:
	VectorGridTest(){ m_Vals = std::vector<T>{10, 20, 1}; }
	std::vector<T> m_Vals;
};

template <>
class VectorGridTest<std::string> : public ::testing::Test
{
public:
	VectorGridTest(){ m_Vals = std::vector<std::string>{"foo", "bar", "testing"}; }
	std::vector<std::string> m_Vals;
};

typedef ::testing::Types<char, int, unsigned int, float, std::string> MyTypes;
TYPED_TEST_CASE(VectorGridTest, MyTypes);

TYPED_TEST(VectorGridTest, ColumnInsert)
{
	VectorGrid<TypeParam> grid;
	grid.InsertColumn(0);
	EXPECT_EQ(grid.ColumnCount(), 1);

	grid.InsertColumn(0);
	EXPECT_EQ(grid.ColumnCount(), 2);

	grid.RemoveColumn(0);
	EXPECT_EQ(grid.ColumnCount(), 1);

	grid.InsertColumn(1);
	grid.Clear();
	EXPECT_EQ(grid.ColumnCount(), 0);
}

TYPED_TEST(VectorGridTest, ElementInsert)
{
	VectorGrid<TypeParam> grid;
	grid.InsertColumn(0);
	grid.InsertColumn(1);

	grid.InsertElement(0, 0, this->m_Vals[0]);
	EXPECT_EQ(grid[0][0], this->m_Vals[0]);

	grid.InsertElement(0, 0, this->m_Vals[1]);
	EXPECT_EQ(grid[0][0], this->m_Vals[1]);

	grid.RemoveElement(0, 0);
	EXPECT_EQ(grid[0][0], this->m_Vals[0]);

	grid.InsertElement(1, 0, this->m_Vals[2]);
	EXPECT_EQ(grid[1][0], this->m_Vals[2]);

	grid.RemoveColumn(0);
	EXPECT_EQ(grid[0][0], this->m_Vals[2]);
}

} // namespace w3