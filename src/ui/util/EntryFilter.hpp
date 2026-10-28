#pragma once

#include "../../core/model/MediaCategory.hpp"

#include <optional>
#include <string>

struct EntryFilter
{
    MediaCategory category;
    std::optional<unsigned> consumedMonth;
    std::optional<unsigned> consumedYear;
    std::string titleQuery;
};
