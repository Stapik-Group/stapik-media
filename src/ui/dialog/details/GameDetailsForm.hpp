#pragma once
#include "DetailsFormWidget.hpp"
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

class GameDetailsForm : public DetailsFormWidget
{
public:
    GameDetailsForm();

    void loadFrom(const MediaDetails& details) override;
    [[nodiscard]] MediaDetails toDetails() const override;

private:
    static constexpr int FIELD_SPACING = 6;

    Gtk::Label m_studioLabel;
    Gtk::Entry m_studioEntry;
    Gtk::Label m_publisherLabel;
    Gtk::Entry m_publisherEntry;
    Gtk::Label m_platformLabel;
    Gtk::Entry m_platformEntry;

    void initLayout();
};