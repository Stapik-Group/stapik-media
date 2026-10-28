#pragma once

#include "../widget/MediaEntryRow.hpp"

#include <gtkmm/box.h>
#include "stapik/ui/widget/EmptyState.hpp"

#include <sigc++/signal.h>
#include <vector>
#include <memory>

class MediaListView : public Gtk::Box
{
public:
    MediaListView();

    void refresh(const std::vector<MediaEntry>& allEntries, const std::vector<std::size_t>& indicesToShow);
    void refreshLabels();
    void setEditable(bool editable);

    sigc::signal<void(std::size_t)>& signalEditRequested();
    sigc::signal<void(std::size_t)>& signalDeleteRequested();

private:
    static constexpr int LIST_SPACING = 2;

    std::vector<std::unique_ptr<MediaEntryRow>> m_rows;
    bool m_editable = true;
    EmptyState m_emptyState;

    sigc::signal<void(std::size_t)> m_signalEditRequested;
    sigc::signal<void(std::size_t)> m_signalDeleteRequested;

    void clearRows();
};