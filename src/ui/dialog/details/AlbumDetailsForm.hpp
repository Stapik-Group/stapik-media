#pragma once
#include "DetailsFormWidget.hpp"
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

class AlbumDetailsForm : public DetailsFormWidget
{
public:
    AlbumDetailsForm();

    void loadFrom(const MediaDetails& details) override;
    [[nodiscard]] MediaDetails toDetails() const override;

private:
    static constexpr int FIELD_SPACING = 6;

    Gtk::Label m_performerLabel;
    Gtk::Entry m_performerEntry;
    Gtk::Label m_publisherLabel;
    Gtk::Entry m_publisherEntry;
    Gtk::Label m_genreLabel;
    Gtk::Entry m_genreEntry;

    void initLayout();
};