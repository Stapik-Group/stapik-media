#pragma once

#include "MediaCategory.hpp"
#include "ReleaseDate.hpp"

#include "details/ScreenDetails.hpp"
#include "details/AlbumDetails.hpp"
#include "details/BookDetails.hpp"
#include "details/GameDetails.hpp"

#include "stapik/domain/YearMonth.hpp"

#include <string>
#include <variant>

using MediaDetails = std::variant<ScreenDetails, BookDetails, AlbumDetails, GameDetails>;

struct MediaEntry {
    MediaCategory category;
    std::string title;
    ReleaseDate releaseDate;
    stapik::domain::YearMonth consumed;
    MediaDetails details;
    std::string id;
};