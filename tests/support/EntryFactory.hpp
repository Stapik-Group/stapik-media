#pragma once

#include "core/model/MediaEntry.hpp"

#include <chrono>
#include <string>
#include <utility>

namespace test
{
    inline ReleaseDate releaseOn(const int year, const unsigned month, const unsigned day)
    {
        return ReleaseDate{
            std::chrono::year{year} / std::chrono::month{month} / std::chrono::day{day},
            DatePrecision::Day
        };
    }

    inline MediaEntry makeEntry(
        const MediaCategory category,
        std::string title,
        const int consumedYear,
        const int consumedMonth,
        MediaDetails details,
        std::string id = "entry-1")
    {
        return MediaEntry{
            category,
            std::move(title),
            releaseOn(2010, 3, 5),
            stapik::domain::YearMonth(consumedYear, consumedMonth),
            std::move(details),
            std::move(id)
        };
    }

    inline MediaEntry makeMovie(std::string title, const int consumedYear, const int consumedMonth, std::string id = "movie-1")
    {
        return makeEntry(MediaCategory::Movie, std::move(title), consumedYear, consumedMonth, ScreenDetails{"Director", "Drama"}, std::move(id));
    }

    inline MediaEntry makeBook(std::string title, const int consumedYear, const int consumedMonth, std::string id = "book-1")
    {
        return makeEntry(MediaCategory::Book, std::move(title), consumedYear, consumedMonth, BookDetails{"Author", "Fantasy", true}, std::move(id));
    }
}
