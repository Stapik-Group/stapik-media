#pragma once

#include "../../core/model/MediaEntry.hpp"
#include "EntryFilter.hpp"
#include <vector>
#include <set>

namespace EntryFilterService
{
    [[nodiscard]] std::vector<std::size_t> filterIndices(const std::vector<MediaEntry>& entries, const EntryFilter& filter);
    [[nodiscard]] std::set<unsigned> distinctConsumedYears(const std::vector<MediaEntry>& entries);
}
