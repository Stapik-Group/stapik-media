#include "core/export/EntryCsv.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace
{
    std::vector<std::string> header()
    {
        return { "category", "title", "released", "consumed", "creator", "publisher", "genre", "platform", "audiobook", "rating", "note" };
    }

    MediaEntry withReleaseDate(MediaEntry entry, const ReleaseDate& releaseDate)
    {
        entry.releaseDate = releaseDate;
        return entry;
    }
}

TEST(EntryCsvTest, FirstRowIsTheHeader)
{
    const auto rows = EntryCsv::table({}, header());

    ASSERT_EQ(rows.size(), 1U);
    EXPECT_EQ(rows[0], header());
}

TEST(EntryCsvTest, HeaderHasAKeyForEveryColumn)
{
    EXPECT_EQ(EntryCsv::HEADER_KEYS.size(), header().size());
}

TEST(EntryCsvTest, ScreenEntryFillsDirectorAndGenre)
{
    const auto rows = EntryCsv::table({ test::makeMovie("Heat", 2024, 3, "a") }, header());

    ASSERT_EQ(rows.size(), 2U);
    EXPECT_EQ(rows[1], (stapik::csv::CsvRow{ "movie", "Heat", "2010-03-05", "2024-03", "Director", "", "Drama", "", "", "", "" }));
}

TEST(EntryCsvTest, BookEntryMarksAudiobooks)
{
    const auto rows = EntryCsv::table({ test::makeBook("Dune", 2023, 11, "b") }, header());

    EXPECT_EQ(rows.at(1), (stapik::csv::CsvRow{ "book", "Dune", "2010-03-05", "2023-11", "Author", "", "Fantasy", "", "true", "", "" }));
}

TEST(EntryCsvTest, AlbumEntryFillsPerformerAndPublisher)
{
    const auto entry = test::makeEntry(MediaCategory::Album, "Kind of Blue", 2022, 1, AlbumDetails{ "Miles Davis", "Columbia", "Jazz" });

    EXPECT_EQ(EntryCsv::table({ entry }, header()).at(1), (stapik::csv::CsvRow{ "album", "Kind of Blue", "2010-03-05", "2022-01", "Miles Davis", "Columbia", "Jazz", "", "", "", "" }));
}

TEST(EntryCsvTest, GameEntryFillsStudioPublisherAndPlatform)
{
    const auto entry = test::makeEntry(MediaCategory::Game, "Portal", 2021, 12, GameDetails{ "Valve", "Valve", "PC" });

    EXPECT_EQ(EntryCsv::table({ entry }, header()).at(1), (stapik::csv::CsvRow{ "game", "Portal", "2010-03-05", "2021-12", "Valve", "Valve", "", "PC", "", "", "" }));
}

TEST(EntryCsvTest, ReleaseDateIsCutToItsPrecision)
{
    const auto base = test::makeMovie("Heat", 2024, 3, "a");
    const auto monthOnly = withReleaseDate(base, ReleaseDate{ std::chrono::year{ 1995 } / std::chrono::month{ 12 } / std::chrono::day{ 1 }, DatePrecision::Month });
    const auto yearOnly = withReleaseDate(base, ReleaseDate{ std::chrono::year{ 1995 } / std::chrono::month{ 1 } / std::chrono::day{ 1 }, DatePrecision::Year });

    const auto rows = EntryCsv::table({ monthOnly, yearOnly }, header());

    EXPECT_EQ(rows.at(1).at(2), "1995-12");
    EXPECT_EQ(rows.at(2).at(2), "1995");
}

TEST(EntryCsvTest, EntriesKeepTheirOrder)
{
    const auto rows = EntryCsv::table({ test::makeMovie("B", 2024, 3, "b"), test::makeMovie("A", 2024, 3, "a") }, header());

    ASSERT_EQ(rows.size(), 3U);
    EXPECT_EQ(rows[1][1], "B");
    EXPECT_EQ(rows[2][1], "A");
}


TEST(EntryCsvTest, RatingAndNoteAreTheLastColumns)
{
    auto entry = test::makeMovie("Heat", 2024, 3, "a");
    entry.rating = 9;
    entry.note = "Great\nfinale";

    const auto row = EntryCsv::table({ entry }, header()).at(1);

    EXPECT_EQ(row.at(9), "9");
    EXPECT_EQ(row.at(10), "Great\nfinale");
}

TEST(EntryCsvTest, MissingRatingAndNoteAreEmpty)
{
    const auto row = EntryCsv::table({ test::makeMovie("Heat", 2024, 3, "a") }, header()).at(1);

    EXPECT_EQ(row.at(9), "");
    EXPECT_EQ(row.at(10), "");
}
