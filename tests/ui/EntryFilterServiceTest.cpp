#include "ui/util/EntryFilterService.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

namespace
{
    std::vector<MediaEntry> sampleEntries()
    {
        return {
            test::makeMovie("Heat", 2024, 3, "a"),
            test::makeBook("Dune", 2024, 3, "b"),
            test::makeMovie("Ronin", 2023, 3, "c"),
            test::makeMovie("Alien", 2024, 11, "d")
        };
    }

    EntryFilter movies()
    {
        return EntryFilter{MediaCategory::Movie, std::nullopt, std::nullopt};
    }
}

TEST(EntryFilterServiceTest, FiltersByCategory)
{
    const auto result = EntryFilterService::filterIndices(sampleEntries(), movies());

    EXPECT_EQ(result, (std::vector<std::size_t>{0, 2, 3}));
}

TEST(EntryFilterServiceTest, FiltersByConsumedYear)
{
    auto filter = movies();
    filter.consumedYear = 2024;

    const auto result = EntryFilterService::filterIndices(sampleEntries(), filter);

    EXPECT_EQ(result, (std::vector<std::size_t>{0, 3}));
}

TEST(EntryFilterServiceTest, FiltersByConsumedMonthAcrossYears)
{
    auto filter = movies();
    filter.consumedMonth = 3;

    const auto result = EntryFilterService::filterIndices(sampleEntries(), filter);

    EXPECT_EQ(result, (std::vector<std::size_t>{0, 2}));
}

TEST(EntryFilterServiceTest, CombinesYearAndMonth)
{
    auto filter = movies();
    filter.consumedYear = 2024;
    filter.consumedMonth = 11;

    const auto result = EntryFilterService::filterIndices(sampleEntries(), filter);

    EXPECT_EQ(result, (std::vector<std::size_t>{3}));
}

TEST(EntryFilterServiceTest, NoMatchesGivesAnEmptyList)
{
    auto filter = movies();
    filter.consumedYear = 1999;

    EXPECT_TRUE(EntryFilterService::filterIndices(sampleEntries(), filter).empty());
    EXPECT_TRUE(EntryFilterService::filterIndices({}, movies()).empty());
}
