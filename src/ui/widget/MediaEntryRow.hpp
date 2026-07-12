#pragma once

#include "../../core/model/MediaEntry.hpp"

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <sigc++/signal.h>

class MediaEntryRow : public Gtk::Box
{
public:
    explicit MediaEntryRow(const MediaEntry& entry);

    void refreshLabels();

    sigc::signal<void()>& signalEditRequested();
    sigc::signal<void()>& signalDeleteRequested();

private:
    static constexpr int ROW_SPACING = 4;
    static constexpr int ROW_MARGIN = 8;
    static constexpr int COLUMN_SPACING = 12;

    Gtk::Box m_infoBox;
    Gtk::Label m_titleLabel;
    Gtk::Label m_detailsLabel;
    Gtk::Label m_datesLabel;

    Gtk::Box m_buttonsBox;
    Gtk::Button m_editButton;
    Gtk::Button m_deleteButton;

    sigc::signal<void()> m_signalEditRequested;
    sigc::signal<void()> m_signalDeleteRequested;

    void initLayout(const MediaEntry& entry);
    [[nodiscard]] static std::string formatDetailsLine(const MediaEntry& entry);
};
