#include "core/model/YearPartitioning.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

TEST(YearPartitioningTest, EntriesFromEarlierYearsAreArchived)
{
    const auto year = YearPartitioning::archiveYearOf(test::makeMovie("Heat", 2023, 5), 2026);

    ASSERT_TRUE(year.has_value());
    EXPECT_EQ(*year, 2023);
}

TEST(YearPartitioningTest, CurrentAndFutureYearsStayInTheMainDocument)
{
    EXPECT_FALSE(YearPartitioning::archiveYearOf(test::makeMovie("Heat", 2026, 1), 2026).has_value());
    EXPECT_FALSE(YearPartitioning::archiveYearOf(test::makeMovie("Heat", 2027, 12), 2026).has_value());
}

TEST(YearPartitioningTest, SameContentIgnoresTheId)
{
    const auto first = test::makeMovie("Heat", 2023, 5, "id-a");
    const auto second = test::makeMovie("Heat", 2023, 5, "id-b");

    EXPECT_TRUE(YearPartitioning::hasSameContent(first, second));
}

TEST(YearPartitioningTest, DifferentContentIsNotTheSame)
{
    const auto movie = test::makeMovie("Heat", 2023, 5);

    EXPECT_FALSE(YearPartitioning::hasSameContent(movie, test::makeMovie("Ronin", 2023, 5)));
    EXPECT_FALSE(YearPartitioning::hasSameContent(movie, test::makeMovie("Heat", 2023, 6)));
    EXPECT_FALSE(YearPartitioning::hasSameContent(movie, test::makeBook("Heat", 2023, 5)));
}

TEST(YearPartitioningTest, ContainsSameContentSearchesTheWholeList)
{
    const std::vector<MediaEntry> entries = { test::makeMovie("Ronin", 2023, 2, "a"), test::makeMovie("Heat", 2023, 5, "b") };

    EXPECT_TRUE(YearPartitioning::containsSameContent(entries, test::makeMovie("Heat", 2023, 5, "other")));
    EXPECT_FALSE(YearPartitioning::containsSameContent(entries, test::makeMovie("Alien", 2023, 5)));
    EXPECT_FALSE(YearPartitioning::containsSameContent({}, test::makeMovie("Heat", 2023, 5)));
}
