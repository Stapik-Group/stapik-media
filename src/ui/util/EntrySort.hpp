#pragma once

#include <optional>
#include <string>
#include <string_view>

enum class SortField { Title, ReleaseDate, Consumed };
enum class SortDirection { Ascending, Descending };

struct EntrySort
{
    SortField field = SortField::Consumed;
    SortDirection direction = SortDirection::Descending;

    [[nodiscard]] bool operator==(const EntrySort&) const = default;
};

[[nodiscard]] inline std::string sortFieldToKey(const SortField field)
{
    switch (field)
    {
        using enum SortField;
        case Title: return "title";
        case ReleaseDate: return "releaseDate";
        case Consumed: return "consumed";
    }
    return "consumed";
}

[[nodiscard]] inline std::optional<SortField> sortFieldFromKey(const std::string_view key)
{
    using enum SortField;
    if (key == "title") return Title;
    if (key == "releaseDate") return ReleaseDate;
    if (key == "consumed") return Consumed;
    return std::nullopt;
}

[[nodiscard]] inline std::string sortDirectionToKey(const SortDirection direction)
{
    return direction == SortDirection::Ascending ? "ascending" : "descending";
}

[[nodiscard]] inline std::optional<SortDirection> sortDirectionFromKey(const std::string_view key)
{
    if (key == "ascending") return SortDirection::Ascending;
    if (key == "descending") return SortDirection::Descending;
    return std::nullopt;
}
