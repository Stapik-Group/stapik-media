#include "EntryCsvImport.hpp"

#include "../model/MediaCategoryKey.hpp"

#include "stapik/domain/YearMonth.hpp"
#include "stapik/text/Trim.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <stdexcept>
#include <string_view>
#include <system_error>

namespace
{
    constexpr std::size_t CATEGORY_COLUMN = 0;
    constexpr std::size_t TITLE_COLUMN = 1;
    constexpr std::size_t RELEASE_DATE_COLUMN = 2;
    constexpr std::size_t CONSUMED_COLUMN = 3;
    constexpr std::size_t CREATOR_COLUMN = 4;
    constexpr std::size_t PUBLISHER_COLUMN = 5;
    constexpr std::size_t GENRE_COLUMN = 6;
    constexpr std::size_t PLATFORM_COLUMN = 7;
    constexpr std::size_t AUDIOBOOK_COLUMN = 8;
    constexpr std::size_t REQUIRED_COLUMNS = CONSUMED_COLUMN + 1;

    constexpr std::size_t YEAR_LENGTH = 4;
    constexpr std::size_t YEAR_MONTH_LENGTH = 7;
    constexpr std::size_t DATE_LENGTH = 10;

    std::string lowered(std::string text)
    {
        std::ranges::transform(text, text.begin(), [](const unsigned char character) { return static_cast<char>(std::tolower(character)); });
        return text;
    }

    std::string cell(const stapik::csv::CsvRow& row, const std::size_t column)
    {
        return column < row.size() ? stapik::text::trim(row[column]) : std::string{};
    }

    bool parseNumber(const std::string_view text, int& value)
    {
        const auto* end = text.data() + text.size();
        const auto [position, error] = std::from_chars(text.data(), end, value);
        return error == std::errc{} && position == end;
    }

    std::optional<ReleaseDate> parseReleaseDate(const std::string& text)
    {
        auto precision = DatePrecision::Day;
        auto iso = text;

        switch (text.size())
        {
            case YEAR_LENGTH:
                precision = DatePrecision::Year;
                iso += "-01-01";
                break;
            case YEAR_MONTH_LENGTH:
                precision = DatePrecision::Month;
                iso += "-01";
                break;
            case DATE_LENGTH:
                break;
            default:
                return std::nullopt;
        }

        try
        {
            return ReleaseDate::fromIsoDateString(iso, precision);
        }
        catch (const std::invalid_argument&)
        {
            return std::nullopt;
        }
    }

    std::optional<stapik::domain::YearMonth> parseConsumed(const std::string& text)
    {
        if (text.size() != YEAR_MONTH_LENGTH || text[YEAR_LENGTH] != '-')
            return std::nullopt;

        int year = 0;
        int month = 0;
        const auto view = std::string_view(text);
        if (!parseNumber(view.substr(0, YEAR_LENGTH), year) || !parseNumber(view.substr(YEAR_LENGTH + 1), month))
            return std::nullopt;

        if (year < stapik::domain::MIN_YEAR_MONTH_YEAR || year > stapik::domain::MAX_YEAR_MONTH_YEAR)
            return std::nullopt;

        if (month < 1 || month > stapik::domain::MONTHS_PER_YEAR)
            return std::nullopt;

        return stapik::domain::YearMonth(year, month);
    }

    std::optional<bool> parseAudiobook(const std::string& text)
    {
        const auto value = lowered(text);
        if (value.empty() || value == "false")
            return false;
        if (value == "true")
            return true;
        return std::nullopt;
    }

    bool isScreen(const MediaCategory category)
    {
        return category == MediaCategory::Movie || category == MediaCategory::Series || category == MediaCategory::Cartoon;
    }

    MediaDetails detailsOf(const MediaCategory category, const stapik::csv::CsvRow& row, const bool audiobook)
    {
        const auto creator = cell(row, CREATOR_COLUMN);
        const auto publisher = cell(row, PUBLISHER_COLUMN);
        const auto genre = cell(row, GENRE_COLUMN);

        if (isScreen(category))
            return ScreenDetails{ creator, genre };
        if (category == MediaCategory::Book)
            return BookDetails{ creator, genre, audiobook };
        if (category == MediaCategory::Album)
            return AlbumDetails{ creator, publisher, genre };
        return GameDetails{ creator, publisher, cell(row, PLATFORM_COLUMN) };
    }

    EntryCsvImport::Row failedRow(const std::size_t number, std::string title, const EntryCsvImport::RowProblem problem)
    {
        return EntryCsvImport::Row{ .number = number, .title = std::move(title), .entry = std::nullopt, .problem = problem };
    }

    EntryCsvImport::Row parseRow(const std::size_t number, const stapik::csv::CsvRow& row)
    {
        using EntryCsvImport::RowProblem;

        auto title = cell(row, TITLE_COLUMN);
        if (row.size() < REQUIRED_COLUMNS)
            return failedRow(number, std::move(title), RowProblem::MissingColumns);

        const auto category = categoryFromKey(lowered(cell(row, CATEGORY_COLUMN)));
        if (!category.has_value())
            return failedRow(number, std::move(title), RowProblem::UnknownCategory);

        if (title.empty())
            return failedRow(number, std::move(title), RowProblem::EmptyTitle);

        const auto releaseDate = parseReleaseDate(cell(row, RELEASE_DATE_COLUMN));
        if (!releaseDate.has_value())
            return failedRow(number, std::move(title), RowProblem::InvalidReleaseDate);

        const auto consumed = parseConsumed(cell(row, CONSUMED_COLUMN));
        if (!consumed.has_value())
            return failedRow(number, std::move(title), RowProblem::InvalidConsumed);

        const auto audiobook = parseAudiobook(cell(row, AUDIOBOOK_COLUMN));
        if (!audiobook.has_value())
            return failedRow(number, std::move(title), RowProblem::InvalidAudiobook);

        MediaEntry entry{ *category, title, *releaseDate, *consumed, detailsOf(*category, row, *audiobook), {} };
        return EntryCsvImport::Row{ .number = number, .title = std::move(title), .entry = std::move(entry), .problem = std::nullopt };
    }

    bool isHeader(const stapik::csv::CsvRow& row)
    {
        return !categoryFromKey(lowered(cell(row, CATEGORY_COLUMN))).has_value();
    }
}

namespace EntryCsvImport
{
    std::vector<Row> parse(const std::vector<stapik::csv::CsvRow>& table)
    {
        std::vector<Row> rows;
        rows.reserve(table.size());

        for (std::size_t index = 0; index < table.size(); ++index)
        {
            if (index == 0 && isHeader(table[index]))
                continue;

            rows.push_back(parseRow(index + 1, table[index]));
        }

        return rows;
    }
}
