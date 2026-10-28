#pragma once

#include "../../core/model/MediaEntry.hpp"

#include <string>
#include <vector>

namespace EntrySuggestions
{
    struct Lists
    {
        std::vector<std::string> creators;
        std::vector<std::string> publishers;
        std::vector<std::string> genres;
        std::vector<std::string> platforms;
    };

    [[nodiscard]] Lists collect(const std::vector<MediaEntry>& entries, MediaCategory category);
}
