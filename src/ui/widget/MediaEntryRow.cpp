#include "MediaEntryRow.hpp"

#include "../util/EntryText.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <pangomm/layout.h>


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

    m_noteLabel.set_text(EntryText::noteLine(entry));
    m_noteLabel.set_halign(Gtk::Align::START);
    m_noteLabel.set_ellipsize(Pango::EllipsizeMode::END);
    m_noteLabel.set_xalign(0.0F);
    m_noteLabel.add_css_class("entry-note");
    m_noteLabel.set_tooltip_text(entry.note);
    m_noteLabel.set_visible(!entry.note.empty());

    m_infoBox.append(m_titleLabel);
    m_infoBox.append(m_detailsLabel);
    m_infoBox.append(m_datesLabel);
    m_infoBox.append(m_noteLabel);
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

void MediaEntryRow::setEditable(const bool editable)
{
    m_editButton.set_sensitive(editable);
    m_deleteButton.set_sensitive(editable);
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