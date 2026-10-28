#include "ui/util/Pagination.hpp"

#include <gtest/gtest.h>

TEST(PaginationTest, EmptyListHasOneEmptyPage)
{
    const auto slice = Pagination::sliceFor(0, 1, 10);

    EXPECT_EQ(slice.page, 1U);
    EXPECT_EQ(slice.totalPages, 1U);
    EXPECT_EQ(slice.first, 0U);
    EXPECT_EQ(slice.last, 0U);
}

TEST(PaginationTest, ShortListFitsOnOnePage)
{
    const auto slice = Pagination::sliceFor(7, 1, 10);

    EXPECT_EQ(slice.totalPages, 1U);
    EXPECT_EQ(slice.first, 0U);
    EXPECT_EQ(slice.last, 7U);
}

TEST(PaginationTest, ExactMultipleDoesNotAddAnEmptyPage)
{
    const auto slice = Pagination::sliceFor(20, 2, 10);

    EXPECT_EQ(slice.totalPages, 2U);
    EXPECT_EQ(slice.first, 10U);
    EXPECT_EQ(slice.last, 20U);
}

TEST(PaginationTest, LastPageHoldsTheRemainder)
{
    const auto slice = Pagination::sliceFor(23, 3, 10);

    EXPECT_EQ(slice.page, 3U);
    EXPECT_EQ(slice.totalPages, 3U);
    EXPECT_EQ(slice.first, 20U);
    EXPECT_EQ(slice.last, 23U);
}

TEST(PaginationTest, PageBeyondTheEndIsClampedToTheLastPage)
{
    const auto slice = Pagination::sliceFor(23, 9, 10);

    EXPECT_EQ(slice.page, 3U);
    EXPECT_EQ(slice.first, 20U);
}

TEST(PaginationTest, PageZeroIsTreatedAsTheFirstPage)
{
    const auto slice = Pagination::sliceFor(23, 0, 10);

    EXPECT_EQ(slice.page, 1U);
    EXPECT_EQ(slice.first, 0U);
    EXPECT_EQ(slice.last, 10U);
}

TEST(PaginationTest, ZeroPageSizeIsTreatedAsOne)
{
    const auto slice = Pagination::sliceFor(3, 2, 0);

    EXPECT_EQ(slice.totalPages, 3U);
    EXPECT_EQ(slice.first, 1U);
    EXPECT_EQ(slice.last, 2U);
}
