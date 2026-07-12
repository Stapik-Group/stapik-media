#include "BookDetailsForm.hpp"
#include "stapik/locale/LocaleManager.hpp"

BookDetailsForm::BookDetailsForm()
{
    initLayout();
}

void BookDetailsForm::initLayout()
{
    set_orientation(Gtk::Orientation::VERTICAL);
    set_spacing(FIELD_SPACING);
    add_css_class("details-form");

    const auto& loc = LocaleManager::instance();
    m_authorLabel.set_text(loc.translate("dialog.entry.author.label"));
    m_authorLabel.set_halign(Gtk::Align::START);
    m_genreLabel.set_text(loc.translate("dialog.entry.genre.label"));
    m_genreLabel.set_halign(Gtk::Align::START);
    m_audiobookCheck.set_label(loc.translate("dialog.entry.audiobook.label"));

    append(m_authorLabel);
    append(m_authorEntry);
    append(m_genreLabel);
    append(m_genreEntry);
    append(m_audiobookCheck);
}

void BookDetailsForm::loadFrom(const MediaDetails& details)
{
    if (const auto* book = std::get_if<BookDetails>(&details))
    {
        m_authorEntry.set_text(book->author);
        m_genreEntry.set_text(book->genre);
        m_audiobookCheck.set_active(book->isAudiobook);
    }
}

MediaDetails BookDetailsForm::toDetails() const
{
    return BookDetails{
        std::string(m_authorEntry.get_text()),
        std::string(m_genreEntry.get_text()),
        m_audiobookCheck.get_active()
    };
}