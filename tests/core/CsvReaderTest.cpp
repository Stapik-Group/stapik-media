#include "core/export/CsvReader.hpp"

#include <gtest/gtest.h>

using stapik::csv::CsvRow;

TEST(CsvReaderTest, ReadsCommaSeparatedRows)
{
    const auto rows = CsvReader::parse("a,b,c\r\n1,2,3\r\n");

    ASSERT_TRUE(rows.has_value());
    EXPECT_EQ(*rows, (std::vector<CsvRow>{ { "a", "b", "c" }, { "1", "2", "3" } }));
}

TEST(CsvReaderTest, DetectsTheSemicolonSeparator)
{
    const auto rows = CsvReader::parse("a;b;c\n1;2,5;3\n");

    ASSERT_TRUE(rows.has_value());
    EXPECT_EQ(*rows, (std::vector<CsvRow>{ { "a", "b", "c" }, { "1", "2,5", "3" } }));
}

TEST(CsvReaderTest, IgnoresTheByteOrderMark)
{
    const auto rows = CsvReader::parse("\xEF\xBB\xBF" "a,b\n");

    ASSERT_TRUE(rows.has_value());
    EXPECT_EQ(*rows, (std::vector<CsvRow>{ { "a", "b" } }));
}

TEST(CsvReaderTest, ReadsQuotedFieldsWithSeparatorsQuotesAndLineBreaks)
{
    const auto rows = CsvReader::parse("\"a,b\",\"say \"\"hi\"\"\",\"line1\r\nline2\"\n");

    ASSERT_TRUE(rows.has_value());
    EXPECT_EQ(*rows, (std::vector<CsvRow>{ { "a,b", "say \"hi\"", "line1\r\nline2" } }));
}

TEST(CsvReaderTest, KeepsEmptyFields)
{
    const auto rows = CsvReader::parse("a,,c\n,,\n");

    ASSERT_TRUE(rows.has_value());
    EXPECT_EQ(*rows, (std::vector<CsvRow>{ { "a", "", "c" }, { "", "", "" } }));
}

TEST(CsvReaderTest, SkipsBlankLinesAndAcceptsAMissingFinalLineBreak)
{
    const auto rows = CsvReader::parse("a,b\n\n\r\nc,d");

    ASSERT_TRUE(rows.has_value());
    EXPECT_EQ(*rows, (std::vector<CsvRow>{ { "a", "b" }, { "c", "d" } }));
}

TEST(CsvReaderTest, EmptyContentHasNoRows)
{
    const auto rows = CsvReader::parse("");

    ASSERT_TRUE(rows.has_value());
    EXPECT_TRUE(rows->empty());
}

TEST(CsvReaderTest, RejectsAQuotedFieldThatIsNeverClosed)
{
    EXPECT_FALSE(CsvReader::parse("a,\"b\nc,d\n").has_value());
}
