#include "EntryText.hpp"

#include "DateFormatter.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <format>
#include <type_traits>
#include <variant>

namespace EntryText
{
    std::string datesLine(const MediaEntry& entry)
    {
        const auto& loc = LocaleManager::instance();

        return std::format("{}: {}   {}: {}",
            loc.translate("list.entry.released"),
            DateFormatter::formatReleaseDate(entry.releaseDate),
            loc.translate("list.entry.consumed"),
            DateFormatter::formatConsumedDate(entry.consumed));
    }

    std::string detailsLine(const MediaEntry& entry)
    {
        const auto& loc = LocaleManager::instance();

        return std::visit([&loc]<typename T>(const T& d) -> std::string
        {
            if constexpr (std::is_same_v<T, ScreenDetails>)
                return loc.translate("dialog.entry.director.label") + ": " + d.director +
                       "   " + loc.translate("dialog.entry.genre.label") + ": " + d.genre;
            else if constexpr (std::is_same_v<T, BookDetails>)
                return loc.translate("dialog.entry.author.label") + ": " + d.author +
                       "   " + loc.translate("dialog.entry.genre.label") + ": " + d.genre +
                       (d.isAudiobook ? "   (" + loc.translate("dialog.entry.audiobook.label") + ")" : "");
            else if constexpr (std::is_same_v<T, AlbumDetails>)
                return loc.translate("dialog.entry.performer.label") + ": " + d.performer +
                       "   " + loc.translate("dialog.entry.publisher.label") + ": " + d.publisher +
                       "   " + loc.translate("dialog.entry.genre.label") + ": " + d.genre;
            else if constexpr (std::is_same_v<T, GameDetails>)
                return loc.translate("dialog.entry.studio.label") + ": " + d.studio +
                       "   " + loc.translate("dialog.entry.publisher.label") + ": " + d.publisher +
                       "   " + loc.translate("dialog.entry.platform.label") + ": " + d.platform;
            return {};
        }, entry.details);
    }
}
