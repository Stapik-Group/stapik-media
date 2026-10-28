#pragma once

#include "../../core/model/MediaEntry.hpp"

#include <string>

namespace EntryText
{
    [[nodiscard]] std::string datesLine(const MediaEntry& entry);
    [[nodiscard]] std::string detailsLine(const MediaEntry& entry);
}
