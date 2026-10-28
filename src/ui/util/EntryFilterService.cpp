#include "EntryFilterService.hpp"

#include "stapik/text/TextFolding.hpp"

#include <algorithm>
#include <chrono>
#include <string>
#include <utility>

namespace
{
    template <typename Key, typename KeyOf>
    void sortByKey(std::vector<std::size_t>& indices, const bool descending, KeyOf keyOf)
    {
        std::vector<std::pair<Key, std::size_t>> keyed;
        keyed.reserve(indices.size());
        for (const auto index : indices)
            keyed.emplace_back(keyOf(index), index);

        std::ranges::sort(keyed, [descending](const auto& left, const auto& right)
        {
            return descending ? right < left : left < right;
        });

        std::ranges::transform(keyed, indices.begin(), [](const auto& item) { return item.second; });
    }
}

namespace EntryFilterService
{
    std::vector<std::size_t> filterIndices(const std::vector<MediaEntry>& entries, const EntryFilter& filter)
    {
        std::vector<std::size_t> result;
        const auto query = stapik::text::foldForSearch(filter.titleQuery);

        for (std::size_t i = 0; i < entries.size(); ++i)
        {
            const auto& entry = entries[i];

            if (entry.category != filter.category)
                continue;

            if (filter.consumedYear.has_value() && entry.consumed.year() != static_cast<int>(filter.consumedYear.value()))
                continue;

            if (filter.consumedMonth.has_value() && entry.consumed.month() != static_cast<int>(filter.consumedMonth.value()))
                continue;

            if (!query.empty() && stapik::text::foldForSearch(entry.title).find(query) == std::string::npos)
                continue;

            result.push_back(i);
        }

        return result;
    }

    void sortIndices(const std::vector<MediaEntry>& entries, std::vector<std::size_t>& indices, const EntrySort& sort)
    {
        const auto descending = sort.direction == SortDirection::Descending;

        switch (sort.field)
        {
            using enum SortField;
            case Title:
                sortByKey<std::string>(indices, descending, [&entries](const std::size_t index) { return stapik::text::foldForSearch(entries[index].title); });
                break;
            case ReleaseDate:
                sortByKey<std::chrono::year_month_day>(indices, descending, [&entries](const std::size_t index) { return entries[index].releaseDate.date; });
                break;
            case Consumed:
                sortByKey<stapik::domain::YearMonth>(indices, descending, [&entries](const std::size_t index) { return entries[index].consumed; });
                break;
        }
    }
}
