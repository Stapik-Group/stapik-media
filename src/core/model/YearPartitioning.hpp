#pragma once

#include "MediaEntry.hpp"

#include <optional>
#include <vector>

namespace YearPartitioning
{
    [[nodiscard]] std::optional<int> archiveYearOf(const MediaEntry& entry, int currentYear);
    [[nodiscard]] bool hasSameContent(const MediaEntry& left, const MediaEntry& right);
    [[nodiscard]] bool containsSameContent(const std::vector<MediaEntry>& entries, const MediaEntry& entry);
}
