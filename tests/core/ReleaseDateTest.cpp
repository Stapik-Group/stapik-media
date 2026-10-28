#include "core/model/ReleaseDate.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <tuple>

TEST(ReleaseDateTest, RoundTripsThroughItsIsoText)
{
    const ReleaseDate date{ std::chrono::year{ 1995 } / std::chrono::month{ 12 } / std::chrono::day{ 9 }, DatePrecision::Day };

    EXPECT_EQ(date.toIsoDateString(), "1995-12-09");

    const auto restored = ReleaseDate::fromIsoDateString(date.toIsoDateString(), DatePrecision::Day);
    EXPECT_EQ(restored.date, date.date);
    EXPECT_EQ(restored.precision, DatePrecision::Day);
}

TEST(ReleaseDateTest, PrecisionIsTakenFromTheArgument)
{
    EXPECT_EQ(ReleaseDate::fromIsoDateString("2001-01-01", DatePrecision::Year).precision, DatePrecision::Year);
}

TEST(ReleaseDateTest, LeapDayIsAccepted)
{
    EXPECT_NO_THROW(std::ignore = ReleaseDate::fromIsoDateString("2024-02-29", DatePrecision::Day));
}

TEST(ReleaseDateTest, MalformedTextIsRejected)
{
    for (const std::string text : { "", "2024", "2024-3-5", "2024/03/05", "2024-03-0x", "abcd-ef-gh", "2024-03-05 ", "-024-03-05" })
        EXPECT_THROW(std::ignore = ReleaseDate::fromIsoDateString(text, DatePrecision::Day), std::invalid_argument) << '"' << text << '"';
}

TEST(ReleaseDateTest, DateThatDoesNotExistIsRejected)
{
    for (const std::string text : { "2024-13-01", "2024-00-10", "2024-02-30", "2023-02-29", "2024-04-31", "2024-01-00", "2024-01-32" })
        EXPECT_THROW(std::ignore = ReleaseDate::fromIsoDateString(text, DatePrecision::Day), std::invalid_argument) << text;
}

TEST(ReleaseDateTest, MonthsHaveTheirOwnLength)
{
    EXPECT_EQ(ReleaseDate::daysInMonth(2025, 1), 31);
    EXPECT_EQ(ReleaseDate::daysInMonth(2025, 4), 30);
    EXPECT_EQ(ReleaseDate::daysInMonth(2025, 12), 31);
}

TEST(ReleaseDateTest, FebruaryDependsOnLeapYears)
{
    EXPECT_EQ(ReleaseDate::daysInMonth(2024, 2), 29);
    EXPECT_EQ(ReleaseDate::daysInMonth(2025, 2), 28);
    EXPECT_EQ(ReleaseDate::daysInMonth(1900, 2), 28);
    EXPECT_EQ(ReleaseDate::daysInMonth(2000, 2), 29);
}

TEST(ReleaseDateTest, MonthOutsideTheYearIsRejected)
{
    EXPECT_THROW(std::ignore = ReleaseDate::daysInMonth(2025, 0), std::invalid_argument);
    EXPECT_THROW(std::ignore = ReleaseDate::daysInMonth(2025, 13), std::invalid_argument);
}

TEST(ReleaseDateTest, DatesBeforeTheTwentiethCenturyRoundTrip)
{
    const ReleaseDate date{ std::chrono::year{ 1832 } / std::chrono::month{ 2 } / std::chrono::day{ 29 }, DatePrecision::Day };

    EXPECT_EQ(ReleaseDate::fromIsoDateString(date.toIsoDateString(), DatePrecision::Day).date, date.date);
}
