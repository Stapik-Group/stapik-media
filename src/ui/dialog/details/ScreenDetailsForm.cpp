#include "ScreenDetailsForm.hpp"

#include "stapik/locale/LocaleManager.hpp"

ScreenDetailsForm::ScreenDetailsForm()
{
    initLayout();
}

void ScreenDetailsForm::initLayout()
{
    set_orientation(Gtk::Orientation::VERTICAL);
    set_spacing(FIELD_SPACING);
    add_css_class("details-form");

    const auto& loc = LocaleManager::instance();
    m_directorLabel.set_text(loc.translate("dialog.entry.director.label"));
    m_directorLabel.set_halign(Gtk::Align::START);
    m_genreLabel.set_text(loc.translate("dialog.entry.genre.label"));
    m_genreLabel.set_halign(Gtk::Align::START);

    append(m_directorLabel);
    append(m_directorEntry);
    append(m_genreLabel);
    append(m_genreEntry);
}

void ScreenDetailsForm::loadFrom(const MediaDetails& details)
{
    if (const auto* screen = std::get_if<ScreenDetails>(&details))
    {
        m_directorEntry.set_text(screen->director);
        m_genreEntry.set_text(screen->genre);
    }
}

MediaDetails ScreenDetailsForm::toDetails() const
{
    return ScreenDetails{
        std::string(m_directorEntry.get_text()),
        std::string(m_genreEntry.get_text())
    };
}