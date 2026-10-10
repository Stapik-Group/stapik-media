#include "YearPartitioning.hpp"

#include "../../infrastructure/storage/MediaEntrySerializer.hpp"

#include <algorithm>

namespace
{
    nlohmann::json contentOf(const MediaEntry& entry)
    {
        auto json = MediaEntrySerializer::toJson(entry);
        json.erase("id");
        return json;
    }
}

namespace YearPartitioning
{
    std::optional<int> archiveYearOf(const MediaEntry& entry, const int currentYear)
    {
        if (entry.consumed.year() >= currentYear)
            return std::nullopt;

        return entry.consumed.year();
    }

    bool hasSameContent(const MediaEntry& left, const MediaEntry& right)
    {
        return contentOf(left) == contentOf(right);
    }

    bool containsSameContent(const std::vector<MediaEntry>& entries, const MediaEntry& entry)
    {
        const auto content = contentOf(entry);
        return std::ranges::any_of(entries, [&content](const MediaEntry& candidate)
        {
            return contentOf(candidate) == content;
        });
    }
}
