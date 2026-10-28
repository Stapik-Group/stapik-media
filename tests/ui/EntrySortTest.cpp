#include "ui/util/EntryFilterService.hpp"
#include "ui/util/EntrySort.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

namespace
{
    std::vector<MediaEntry> sampleEntries()
    {
        auto heat = test::makeMovie("heat", 2024, 3, "a");
        heat.releaseDate = test::releaseOn(1995, 12, 15);

        auto alien = test::makeMovie("Alien", 2023, 11, "b");
        alien.releaseDate = test::releaseOn(1979, 5, 25);

        auto zodiac = test::makeMovie("Zodiac", 2024, 3, "c");
        zodiac.releaseDate = test::releaseOn(2007, 3, 2);

        auto ronin = test::makeMovie("Ronin", 2024, 9, "d");
        ronin.releaseDate = test::releaseOn(1998, 9, 25);

        return { heat, alien, zodiac, ronin };
    }

    std::vector<std::string> sortedIds(const EntrySort& sort)
    {
        const auto entries = sampleEntries();
        std::vector<std::size_t> indices{ 0, 1, 2, 3 };
        EntryFilterService::sortIndices(entries, indices, sort);

        std::vector<std::string> ids;
        for (const auto index : indices)
            ids.push_back(entries[index].id);
        return ids;
    }
}

TEST(EntrySortTest, DefaultIsNewestConsumedFirst)
{
    constexpr EntrySort sort;

    EXPECT_EQ(sort.field, SortField::Consumed);
    EXPECT_EQ(sort.direction, SortDirection::Descending);
}

TEST(EntrySortTest, SortsByTitleIgnoringCase)
{
    EXPECT_EQ(sortedIds({ SortField::Title, SortDirection::Ascending }), (std::vector<std::string>{ "b", "a", "d", "c" }));
    EXPECT_EQ(sortedIds({ SortField::Title, SortDirection::Descending }), (std::vector<std::string>{ "c", "d", "a", "b" }));
}

TEST(EntrySortTest, SortsByReleaseDate)
{
    EXPECT_EQ(sortedIds({ SortField::ReleaseDate, SortDirection::Ascending }), (std::vector<std::string>{ "b", "a", "d", "c" }));
    EXPECT_EQ(sortedIds({ SortField::ReleaseDate, SortDirection::Descending }), (std::vector<std::string>{ "c", "d", "a", "b" }));
}

TEST(EntrySortTest, SortsByConsumedMonthWithLaterAddedEntriesFirstWhenDescending)
{
    EXPECT_EQ(sortedIds({ SortField::Consumed, SortDirection::Ascending }), (std::vector<std::string>{ "b", "a", "c", "d" }));
    EXPECT_EQ(sortedIds({ SortField::Consumed, SortDirection::Descending }), (std::vector<std::string>{ "d", "c", "a", "b" }));
}

TEST(EntrySortTest, SortsOnlyTheGivenIndices)
{
    const auto entries = sampleEntries();
    std::vector<std::size_t> indices{ 3, 0 };

    EntryFilterService::sortIndices(entries, indices, { SortField::Title, SortDirection::Ascending });

    EXPECT_EQ(indices, (std::vector<std::size_t>{ 0, 3 }));
}

TEST(EntrySortTest, KeysRoundTrip)
{
    for (const auto field : { SortField::Title, SortField::ReleaseDate, SortField::Consumed })
        EXPECT_EQ(sortFieldFromKey(sortFieldToKey(field)), field);

    for (const auto direction : { SortDirection::Ascending, SortDirection::Descending })
        EXPECT_EQ(sortDirectionFromKey(sortDirectionToKey(direction)), direction);
}

TEST(EntrySortTest, UnknownKeysAreRejected)
{
    EXPECT_FALSE(sortFieldFromKey("added").has_value());
    EXPECT_FALSE(sortDirectionFromKey("").has_value());
}
