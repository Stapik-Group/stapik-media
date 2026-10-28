#pragma once

#include "../../core/model/MediaEntry.hpp"
#include "EntryFilter.hpp"
#include "EntrySort.hpp"
#include <vector>
#include <set>

namespace EntryFilterService
{
    [[nodiscard]] std::vector<std::size_t> filterIndices(const std::vector<MediaEntry>& entries, const EntryFilter& filter);
    void sortIndices(const std::vector<MediaEntry>& entries, std::vector<std::size_t>& indices, const EntrySort& sort);
}
