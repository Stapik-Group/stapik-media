#include "MediaEntryRow.hpp"

#include "../util/EntryText.hpp"

#include "stapik/locale/LocaleManager.hpp"


MediaEntryRow::MediaEntryRow(const MediaEntry& entry) :
    Box(Gtk::Orientation::HORIZONTAL, COLUMN_SPACING),
    m_infoBox(Gtk::Orientation::VERTICAL, ROW_SPACING),
    m_buttonsBox(Gtk::Orientation::HORIZONTAL, ROW_SPACING),
    m_entry(entry)
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

    m_detailsLabel.set_text(EntryText::detailsLine(entry));
    m_detailsLabel.set_halign(Gtk::Align::START);
    m_detailsLabel.add_css_class("entry-details");

    m_datesLabel.set_text(EntryText::datesLine(entry));
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

void MediaEntryRow::refreshLabels()
{
    m_editButton.set_label(LocaleManager::instance().translate("list.entry.edit"));
    m_deleteButton.set_label(LocaleManager::instance().translate("list.entry.delete"));

    m_detailsLabel.set_text(EntryText::detailsLine(m_entry));
    m_datesLabel.set_text(EntryText::datesLine(m_entry));
}

sigc::signal<void()>& MediaEntryRow::signalEditRequested()
{
    return m_signalEditRequested;
}

sigc::signal<void()>& MediaEntryRow::signalDeleteRequested()
{
    return m_signalDeleteRequested;
}