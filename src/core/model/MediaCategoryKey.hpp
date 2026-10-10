#pragma once

#include "MediaCategory.hpp"

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

[[nodiscard]] inline std::string categoryToKey(const MediaCategory category)
{
    switch (category)
    {
        using enum MediaCategory;
        case Movie: return "movie";
        case Series: return "series";
        case Cartoon: return "cartoon";
        case Book: return "book";
        case Album: return "album";
        case Game: return "game";
    }
    throw std::invalid_argument("Unknown MediaCategory");
}

[[nodiscard]] inline std::optional<MediaCategory> categoryFromKey(const std::string_view key)
{
    using enum MediaCategory;
    if (key == "movie") return Movie;
    if (key == "series") return Series;
    if (key == "cartoon") return Cartoon;
    if (key == "book") return Book;
    if (key == "album") return Album;
    if (key == "game") return Game;
    return std::nullopt;
}
