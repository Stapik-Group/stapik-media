#include "AlbumDetailsForm.hpp"
#include "stapik/locale/LocaleManager.hpp"

AlbumDetailsForm::AlbumDetailsForm()
{
    initLayout();
}

void AlbumDetailsForm::initLayout()
{
    set_orientation(Gtk::Orientation::VERTICAL);
    set_spacing(FIELD_SPACING);
    add_css_class("details-form");

    const auto& loc = LocaleManager::instance();
    m_performerLabel.set_text(loc.translate("dialog.entry.performer.label"));
    m_performerLabel.set_halign(Gtk::Align::START);
    m_publisherLabel.set_text(loc.translate("dialog.entry.publisher.label"));
    m_publisherLabel.set_halign(Gtk::Align::START);
    m_genreLabel.set_text(loc.translate("dialog.entry.genre.label"));
    m_genreLabel.set_halign(Gtk::Align::START);

    append(m_performerLabel);
    append(m_performerEntry);
    append(m_publisherLabel);
    append(m_publisherEntry);
    append(m_genreLabel);
    append(m_genreEntry);
}

void AlbumDetailsForm::loadFrom(const MediaDetails& details)
{
    if (const auto* album = std::get_if<AlbumDetails>(&details))
    {
        m_performerEntry.set_text(album->performer);
        m_publisherEntry.set_text(album->publisher);
        m_genreEntry.set_text(album->genre);
    }
}

MediaDetails AlbumDetailsForm::toDetails() const
{
    return AlbumDetails{
        std::string(m_performerEntry.get_text()),
        std::string(m_publisherEntry.get_text()),
        std::string(m_genreEntry.get_text())
    };
}