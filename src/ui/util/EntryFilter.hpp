#pragma once

#include "../../core/model/MediaCategory.hpp"

#include <optional>

struct EntryFilter
{
    MediaCategory category;
    std::optional<unsigned> consumedMonth;
    std::optional<unsigned> consumedYear;
};
