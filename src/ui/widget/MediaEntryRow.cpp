#include "MediaEntryRow.hpp"

#include "../util/DateFormatter.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <variant>

MediaEntryRow::MediaEntryRow(const MediaEntry& entry) :
    Box(Gtk::Orientation::HORIZONTAL, COLUMN_SPACING),
    m_infoBox(Gtk::Orientation::VERTICAL, ROW_SPACING),
    m_buttonsBox(Gtk::Orientation::HORIZONTAL, ROW_SPACING)
{
    initLayout(entry);
}

void MediaEntryRow::initLayout(const MediaEntry& entry)
{
    set_margin(ROW_MARGIN);
    add_css_class("media-entry-row");

    m_titleLabel.set_text(entry.title);
    m_titleLabel.set_halign(Gtk::Align::START);
    m_titleLabel.add_css_class("entry-title");

    m_detailsLabel.set_text(formatDetailsLine(entry));
    m_detailsLabel.set_halign(Gtk::Align::START);
    m_detailsLabel.add_css_class("entry-details");

    const auto datesText =
        LocaleManager::instance().translate("list.entry.released") + ": " + DateFormatter::formatReleaseDate(entry.releaseDate) +
        "   " +
        LocaleManager::instance().translate("list.entry.consumed") + ": " + DateFormatter::formatConsumedDate(entry.consumed);
    m_datesLabel.set_text(datesText);
    m_datesLabel.set_halign(Gtk::Align::START);
    m_datesLabel.add_css_class("entry-dates");

    m_infoBox.append(m_titleLabel);
    m_infoBox.append(m_detailsLabel);
    m_infoBox.append(m_datesLabel);
    m_infoBox.set_hexpand(true);

    m_editButton.set_label(LocaleManager::instance().translate("list.entry.edit"));
    m_editButton.signal_clicked().connect([this] { m_signalEditRequested.emit(); });

    m_deleteButton.set_label(LocaleManager::instance().translate("list.entry.delete"));
    m_deleteButton.signal_clicked().connect([this] { m_signalDeleteRequested.emit(); });

    m_buttonsBox.append(m_editButton);
    m_buttonsBox.append(m_deleteButton);
    m_buttonsBox.set_valign(Gtk::Align::CENTER);

    append(m_infoBox);
    append(m_buttonsBox);
}

std::string MediaEntryRow::formatDetailsLine(const MediaEntry& entry)
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

void MediaEntryRow::refreshLabels()
{
    m_editButton.set_label(LocaleManager::instance().translate("list.entry.edit"));
    m_deleteButton.set_label(LocaleManager::instance().translate("list.entry.delete"));
}

sigc::signal<void()>& MediaEntryRow::signalEditRequested()
{
    return m_signalEditRequested;
}

sigc::signal<void()>& MediaEntryRow::signalDeleteRequested()
{
    return m_signalDeleteRequested;
}