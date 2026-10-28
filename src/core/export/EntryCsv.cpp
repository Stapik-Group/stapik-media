#include "EntryCsv.hpp"

#include "../model/MediaCategoryKey.hpp"

#include <format>
#include <type_traits>
#include <variant>

namespace
{
    struct DetailColumns
    {
        std::string creator;
        std::string publisher;
        std::string genre;
        std::string platform;
        std::string audiobook;
    };

    std::string releaseDateText(const ReleaseDate& releaseDate)
    {
        const auto text = releaseDate.toIsoDateString();

        switch (releaseDate.precision)
        {
            case DatePrecision::Day: return text;
            case DatePrecision::Month: return text.substr(0, 7);
            case DatePrecision::Year: return text.substr(0, 4);
        }
        return text;
    }

    std::string consumedText(const stapik::domain::YearMonth& consumed)
    {
        return std::format("{:04}-{:02}", consumed.year(), consumed.month());
    }

    DetailColumns columnsOf(const MediaDetails& details)
    {
        return std::visit([]<typename T>(const T& item) -> DetailColumns
        {
            if constexpr (std::is_same_v<T, ScreenDetails>)
                return { .creator = item.director, .genre = item.genre };
            else if constexpr (std::is_same_v<T, BookDetails>)
                return { .creator = item.author, .genre = item.genre, .audiobook = item.isAudiobook ? "true" : "false" };
            else if constexpr (std::is_same_v<T, AlbumDetails>)
                return { .creator = item.performer, .publisher = item.publisher, .genre = item.genre };
            else
                return { .creator = item.studio, .publisher = item.publisher, .platform = item.platform };
        }, details);
    }
}

namespace EntryCsv
{
    std::vector<stapik::csv::CsvRow> table(const std::vector<MediaEntry>& entries, const std::vector<std::string>& header)
    {
        std::vector<stapik::csv::CsvRow> rows;
        rows.reserve(entries.size() + 1);
        rows.push_back(header);

        for (const auto& entry : entries)
        {
            const auto columns = columnsOf(entry.details);
            rows.push_back({
                categoryToKey(entry.category),
                entry.title,
                releaseDateText(entry.releaseDate),
                consumedText(entry.consumed),
                columns.creator,
                columns.publisher,
                columns.genre,
                columns.platform,
                columns.audiobook
            });
        }

        return rows;
    }
}
