#include "GameDetailsForm.hpp"
#include "stapik/locale/LocaleManager.hpp"

GameDetailsForm::GameDetailsForm()
{
    initLayout();
}

void GameDetailsForm::initLayout()
{
    set_orientation(Gtk::Orientation::VERTICAL);
    set_spacing(FIELD_SPACING);
    add_css_class("details-form");

    const auto& loc = LocaleManager::instance();
    m_studioLabel.set_text(loc.translate("dialog.entry.studio.label"));
    m_studioLabel.set_halign(Gtk::Align::START);
    m_publisherLabel.set_text(loc.translate("dialog.entry.publisher.label"));
    m_publisherLabel.set_halign(Gtk::Align::START);
    m_platformLabel.set_text(loc.translate("dialog.entry.platform.label"));
    m_platformLabel.set_halign(Gtk::Align::START);

    append(m_studioLabel);
    append(m_studioEntry);
    append(m_publisherLabel);
    append(m_publisherEntry);
    append(m_platformLabel);
    append(m_platformEntry);
}

void GameDetailsForm::loadFrom(const MediaDetails& details)
{
    if (const auto* game = std::get_if<GameDetails>(&details))
    {
        m_studioEntry.set_text(game->studio);
        m_publisherEntry.set_text(game->publisher);
        m_platformEntry.set_text(game->platform);
    }
}

MediaDetails GameDetailsForm::toDetails() const
{
    return GameDetails{
        std::string(m_studioEntry.get_text()),
        std::string(m_publisherEntry.get_text()),
        std::string(m_platformEntry.get_text())
    };
}