#include "core/export/CsvReader.hpp"
#include "core/export/EntryCsv.hpp"
#include "core/export/EntryCsvImport.hpp"
#include "core/model/YearPartitioning.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

#include <variant>

namespace
{
    using EntryCsvImport::RowProblem;
    using stapik::csv::CsvRow;

    const CsvRow HEADER{ "Category", "Title", "Release date", "Consumed", "Creator", "Publisher", "Genre", "Platform", "Audiobook", "Rating", "Note" };

    EntryCsvImport::Row onlyRowOf(const CsvRow& row)
    {
        const auto rows = EntryCsvImport::parse({ HEADER, row });
        EXPECT_EQ(rows.size(), 1U);
        return rows.front();
    }

    RowProblem problemOf(const CsvRow& row)
    {
        const auto parsed = onlyRowOf(row);
        EXPECT_FALSE(parsed.entry.has_value());
        EXPECT_TRUE(parsed.problem.has_value());
        return parsed.problem.value_or(RowProblem::MissingColumns);
    }
}

TEST(EntryCsvImportTest, SkipsTheHeaderInAnyLanguage)
{
    const auto rows = EntryCsvImport::parse({ { "Kategoria", "Tytuł", "Wydano", "Skonsumowano" }, { "movie", "Heat", "1995-12-15", "2024-03" } });

    ASSERT_EQ(rows.size(), 1U);
    EXPECT_EQ(rows.front().number, 2U);
    EXPECT_TRUE(rows.front().entry.has_value());
}

TEST(EntryCsvImportTest, ReadsAFileWithoutAHeader)
{
    const auto rows = EntryCsvImport::parse({ { "movie", "Heat", "1995-12-15", "2024-03" } });

    ASSERT_EQ(rows.size(), 1U);
    EXPECT_EQ(rows.front().number, 1U);
}

TEST(EntryCsvImportTest, ReadsEveryDatePrecision)
{
    const auto day = onlyRowOf({ "movie", "Heat", "1995-12-15", "2024-03" });
    const auto month = onlyRowOf({ "movie", "Heat", "1995-12", "2024-03" });
    const auto year = onlyRowOf({ "movie", "Heat", "1995", "2024-03" });

    EXPECT_EQ(day.entry->releaseDate.precision, DatePrecision::Day);
    EXPECT_EQ(day.entry->releaseDate.toIsoDateString(), "1995-12-15");
    EXPECT_EQ(month.entry->releaseDate.precision, DatePrecision::Month);
    EXPECT_EQ(month.entry->releaseDate.toIsoDateString(), "1995-12-01");
    EXPECT_EQ(year.entry->releaseDate.precision, DatePrecision::Year);
    EXPECT_EQ(year.entry->releaseDate.toIsoDateString(), "1995-01-01");
}

TEST(EntryCsvImportTest, TrimsTheTitleAndAcceptsAnyLetterCaseOfTheCategory)
{
    const auto row = onlyRowOf({ " Movie ", "  Heat  ", "1995", "2024-03" });

    ASSERT_TRUE(row.entry.has_value());
    EXPECT_EQ(row.entry->title, "Heat");
    EXPECT_EQ(row.entry->category, MediaCategory::Movie);
    EXPECT_TRUE(row.entry->id.empty());
}

TEST(EntryCsvImportTest, MissingDetailColumnsAreEmpty)
{
    const auto row = onlyRowOf({ "movie", "Heat", "1995", "2024-03" });

    ASSERT_TRUE(row.entry.has_value());
    EXPECT_EQ(std::get<ScreenDetails>(row.entry->details).director, "");
}

TEST(EntryCsvImportTest, ReportsEveryKindOfProblem)
{
    EXPECT_EQ(problemOf({ "movie", "Heat", "1995" }), RowProblem::MissingColumns);
    EXPECT_EQ(problemOf({ "podcast", "Heat", "1995", "2024-03" }), RowProblem::UnknownCategory);
    EXPECT_EQ(problemOf({ "movie", "  ", "1995", "2024-03" }), RowProblem::EmptyTitle);
    EXPECT_EQ(problemOf({ "movie", "Heat", "1995-13-01", "2024-03" }), RowProblem::InvalidReleaseDate);
    EXPECT_EQ(problemOf({ "movie", "Heat", "yesterday", "2024-03" }), RowProblem::InvalidReleaseDate);
    EXPECT_EQ(problemOf({ "movie", "Heat", "1995", "2024-13" }), RowProblem::InvalidConsumed);
    EXPECT_EQ(problemOf({ "movie", "Heat", "1995", "0000-01" }), RowProblem::InvalidConsumed);
    EXPECT_EQ(problemOf({ "movie", "Heat", "1995", "March 2024" }), RowProblem::InvalidConsumed);
    EXPECT_EQ(problemOf({ "book", "Dune", "1965", "2024-03", "Herbert", "", "", "", "maybe" }), RowProblem::InvalidAudiobook);
    EXPECT_EQ(problemOf({ "movie", "Heat", "1995", "2024-03", "", "", "", "", "", "11" }), RowProblem::InvalidRating);
    EXPECT_EQ(problemOf({ "movie", "Heat", "1995", "2024-03", "", "", "", "", "", "0" }), RowProblem::InvalidRating);
    EXPECT_EQ(problemOf({ "movie", "Heat", "1995", "2024-03", "", "", "", "", "", "good" }), RowProblem::InvalidRating);
}

TEST(EntryCsvImportTest, KeepsTheTitleOfAnInvalidRowForThePreview)
{
    const auto row = onlyRowOf({ "movie", "Heat", "never", "2024-03" });

    EXPECT_EQ(row.title, "Heat");
}

TEST(EntryCsvImportTest, ExportedEntriesAreReadBackUnchanged)
{
    auto rated = test::makeMovie("Heat", 2024, 3, "a");
    rated.rating = 9;
    rated.note = "Great, but long.\nSecond \"line\"";

    const std::vector<MediaEntry> entries{
        rated,
        test::makeBook("Dune; \"Part\" One", 2023, 11, "b"),
        test::makeEntry(MediaCategory::Album, "Thriller", 2022, 1, AlbumDetails{ "Michael Jackson", "Epic", "Pop" }, "c"),
        test::makeEntry(MediaCategory::Game, "Doom", 2021, 12, GameDetails{ "id Software", "Bethesda", "PC" }, "d"),
        test::makeEntry(MediaCategory::Series, "Dark", 2020, 6, ScreenDetails{ "Baran bo Odar", "Thriller" }, "e")
    };

    stapik::csv::CsvWriter writer(stapik::csv::CsvOptions{ .separator = stapik::csv::CsvSeparator::Semicolon, .byteOrderMark = true });
    for (const auto& row : EntryCsv::table(entries, { HEADER.begin(), HEADER.end() }))
        writer.addRow(row);

    const auto table = CsvReader::parse(writer.content());
    ASSERT_TRUE(table.has_value());
    const auto rows = EntryCsvImport::parse(*table);

    ASSERT_EQ(rows.size(), entries.size());
    for (std::size_t index = 0; index < entries.size(); ++index)
    {
        ASSERT_TRUE(rows[index].entry.has_value()) << index;
        const auto& read = *rows[index].entry;
        const auto& original = entries[index];
        EXPECT_TRUE(YearPartitioning::hasSameContent(read, original)) << index;
    }
}

TEST(EntryCsvImportTest, ReadsTheAudiobookFlag)
{
    const auto audiobook = onlyRowOf({ "book", "Dune", "1965", "2024-03", "Herbert", "", "SF", "", "TRUE" });
    const auto paper = onlyRowOf({ "book", "Dune", "1965", "2024-03", "Herbert", "", "SF", "", "false" });

    EXPECT_TRUE(std::get<BookDetails>(audiobook.entry->details).isAudiobook);
    EXPECT_FALSE(std::get<BookDetails>(paper.entry->details).isAudiobook);
}


TEST(EntryCsvImportTest, ReadsTheRatingAndTheNote)
{
    const auto rated = onlyRowOf({ "movie", "Heat", "1995", "2024-03", "", "", "", "", "", "8", "  Worth a rewatch " });
    const auto unrated = onlyRowOf({ "movie", "Heat", "1995", "2024-03", "", "", "", "", "", "", "" });
    const auto short_row = onlyRowOf({ "movie", "Heat", "1995", "2024-03" });

    EXPECT_EQ(rated.entry->rating, 8);
    EXPECT_EQ(rated.entry->note, "Worth a rewatch");
    EXPECT_FALSE(unrated.entry->rating.has_value());
    EXPECT_TRUE(unrated.entry->note.empty());
    EXPECT_FALSE(short_row.entry->rating.has_value());
}
