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

TEST(VectorGridTest, MoveCurrent)
{
	VectorGrid<int> grid;

	GridHead<int> col;
	col.m_Elems = std::vector<int>{0, 1, 2, 3, 4, 5};
	grid.InsertColumn(0, std::move(col));

	col.m_Elems = std::vector<int>{10, 11, 12};
	grid.InsertColumn(1, std::move(col));

	EXPECT_EQ(0, *grid.GetCurrent());

	grid.Move(EGD_RIGHT);
	EXPECT_EQ(10, *grid.GetCurrent());

	grid.Move(EGD_DOWN);
	EXPECT_EQ(11, *grid.GetCurrent());

	grid.Move(EGD_DOWN);
	EXPECT_EQ(12, *grid.GetCurrent());

	grid.Move(EGD_DOWN);
	EXPECT_EQ(10, *grid.GetCurrent());

	grid.Move(EGD_UP);
	EXPECT_EQ(12, *grid.GetCurrent());

	grid.Move(EGD_RIGHT);
	EXPECT_EQ(2, *grid.GetCurrent());

	grid.Move(EGD_DOWN);
	EXPECT_EQ(3, *grid.GetCurrent());

	grid.Move(EGD_RIGHT);
	EXPECT_EQ(12, *grid.GetCurrent());
}

TYPED_TEST(VectorGridTest, CopyAndMove)
{
	VectorGrid<TypeParam> grid;
	grid.InsertColumn(0);
	grid.InsertColumn(1);
	grid.InsertElement(0, 0, this->m_Vals[0]);
	grid.InsertElement(0, 0, this->m_Vals[1]);
	grid.InsertElement(1, 0, this->m_Vals[2]);
	EXPECT_EQ(grid.ColumnCount(), 2);

	VectorGrid<TypeParam> grid2(grid);
	grid2.RemoveColumn(1);
	EXPECT_EQ(grid2.ColumnCount(), 1);

	VectorGrid<TypeParam> grid3(std::move(grid2));
	EXPECT_EQ(grid3.ColumnCount(), 1);
	EXPECT_EQ(grid2.ColumnCount(), 0);

	VectorGrid<TypeParam> grid4;
	EXPECT_EQ(grid4.ColumnCount(), 0);

	grid4 = grid3;
	EXPECT_EQ(grid3.ColumnCount(), 1);
	EXPECT_EQ(grid4.ColumnCount(), 1);

	grid3 = std::move(grid);
	EXPECT_EQ(grid.ColumnCount(), 0);
	EXPECT_EQ(grid3.ColumnCount(), 2);
}

} // namespace w3