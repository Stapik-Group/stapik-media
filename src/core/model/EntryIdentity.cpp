#include "EntryIdentity.hpp"

#include "stapik/text/TextFolding.hpp"
#include "stapik/text/Trim.hpp"

#include <format>

namespace EntryIdentity
{
    std::string titleKey(const std::string& title)
    {
        return stapik::text::foldCase(stapik::text::trim(title));
    }

    std::string workKey(const MediaEntry& entry)
    {
        return std::format("{}|{}|{}", static_cast<int>(entry.category), static_cast<int>(entry.releaseDate.date.year()), titleKey(entry.title));
    }

    bool sameWork(const MediaEntry& left, const MediaEntry& right)
    {
        return left.category == right.category
            && left.releaseDate.date.year() == right.releaseDate.date.year()
            && titleKey(left.title) == titleKey(right.title);
    }
}
