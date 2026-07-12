#include "EntryFilterService.hpp"

namespace EntryFilterService
{
    std::vector<std::size_t> filterIndices(const std::vector<MediaEntry>& entries, const EntryFilter& filter)
    {
        std::vector<std::size_t> result;

        for (std::size_t i = 0; i < entries.size(); ++i)
        {
            const auto& entry = entries[i];

            if (entry.category != filter.category)
                continue;

            if (filter.consumedYear.has_value() && entry.consumed.year != filter.consumedYear.value())
                continue;

            if (filter.consumedMonth.has_value() && entry.consumed.month != filter.consumedMonth.value())
                continue;

            result.push_back(i);
        }

        return result;
    }

    std::set<unsigned> distinctConsumedYears(const std::vector<MediaEntry>& entries)
    {
        std::set<unsigned> years;
        for (const auto& entry : entries)
            years.insert(entry.consumed.year);
        return years;
    }
}