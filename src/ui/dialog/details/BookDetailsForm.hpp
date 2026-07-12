#pragma once

#include "DetailsFormWidget.hpp"

#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/checkbutton.h>

class BookDetailsForm : public DetailsFormWidget
{
public:
    BookDetailsForm();

    void loadFrom(const MediaDetails& details) override;
    [[nodiscard]] MediaDetails toDetails() const override;

private:
    static constexpr int FIELD_SPACING = 6;

    Gtk::Label m_authorLabel;
    Gtk::Entry m_authorEntry;
    Gtk::Label m_genreLabel;
    Gtk::Entry m_genreEntry;
    Gtk::CheckButton m_audiobookCheck;

    void initLayout();
};