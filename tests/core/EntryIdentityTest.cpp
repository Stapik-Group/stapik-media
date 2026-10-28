#include "core/model/EntryIdentity.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

namespace
{
    MediaEntry releasedIn(MediaEntry entry, const int year, const DatePrecision precision = DatePrecision::Day)
    {
        entry.releaseDate = ReleaseDate{ std::chrono::year{ year } / std::chrono::month{ 1 } / std::chrono::day{ 1 }, precision };
        return entry;
    }
}

TEST(EntryIdentityTest, SameTitleInTheSameCategoryAndYearIsTheSameWork)
{
    EXPECT_TRUE(EntryIdentity::sameWork(test::makeMovie("Heat", 2024, 3, "a"), test::makeMovie("Heat", 2020, 11, "b")));
}

TEST(EntryIdentityTest, LetterCaseDoesNotMatter)
{
    EXPECT_TRUE(EntryIdentity::sameWork(test::makeMovie("Heat", 2024, 3, "a"), test::makeMovie("hEAT", 2024, 3, "b")));
    EXPECT_TRUE(EntryIdentity::sameWork(test::makeMovie("Łódź", 2024, 3, "a"), test::makeMovie("łÓDŹ", 2024, 3, "b")));
}

TEST(EntryIdentityTest, SurroundingSpacesDoNotMatter)
{
    EXPECT_TRUE(EntryIdentity::sameWork(test::makeMovie("  Heat ", 2024, 3, "a"), test::makeMovie("Heat", 2024, 3, "b")));
}

TEST(EntryIdentityTest, DiacriticsDoMatter)
{
    EXPECT_FALSE(EntryIdentity::sameWork(test::makeMovie("Leon", 2024, 3, "a"), test::makeMovie("Léon", 2024, 3, "b")));
}

TEST(EntryIdentityTest, SpacesInsideTheTitleDoMatter)
{
    EXPECT_FALSE(EntryIdentity::sameWork(test::makeMovie("Blade Runner", 2024, 3, "a"), test::makeMovie("BladeRunner", 2024, 3, "b")));
}

TEST(EntryIdentityTest, SameTitleReleasedInAnotherYearIsADifferentWork)
{
    const auto original = releasedIn(test::makeMovie("Superman", 2024, 3, "a"), 1978);
    const auto remake = releasedIn(test::makeMovie("Superman", 2024, 3, "b"), 2025);

    EXPECT_FALSE(EntryIdentity::sameWork(original, remake));
}

TEST(EntryIdentityTest, PrecisionOfTheReleaseDateDoesNotMatter)
{
    const auto exact = releasedIn(test::makeMovie("Superman", 2024, 3, "a"), 1978, DatePrecision::Day);
    const auto yearOnly = releasedIn(test::makeMovie("Superman", 2023, 8, "b"), 1978, DatePrecision::Year);

    EXPECT_TRUE(EntryIdentity::sameWork(exact, yearOnly));
}

TEST(EntryIdentityTest, SameTitleInAnotherCategoryIsADifferentIdentity)
{
    EXPECT_FALSE(EntryIdentity::sameWork(test::makeMovie("Dune", 2024, 3, "a"), test::makeBook("Dune", 2024, 3, "b")));
}

TEST(EntryIdentityTest, TitleKeyIsTrimmedAndFolded)
{
    EXPECT_EQ(EntryIdentity::titleKey("  HEAT\t"), "heat");
}
