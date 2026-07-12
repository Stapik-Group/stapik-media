#pragma once

#include "../widget/MediaEntryRow.hpp"
#include "../../core/model/MediaEntry.hpp"

#include <gtkmm/box.h>
#include <sigc++/signal.h>
#include <vector>
#include <memory>

class MediaListView : public Gtk::Box
{
public:
    MediaListView();

    void refresh(const std::vector<MediaEntry>& allEntries, MediaCategory filterCategory);
    void refreshLabels() const;

    sigc::signal<void(std::size_t)>& signalEditRequested();
    sigc::signal<void(std::size_t)>& signalDeleteRequested();

private:
    static constexpr int LIST_SPACING = 2;

    Gtk::Label m_emptyPlaceholder;

    std::vector<std::unique_ptr<MediaEntryRow>> m_rows;
    std::vector<std::size_t> m_visibleIndices;

    sigc::signal<void(std::size_t)> m_signalEditRequested;
    sigc::signal<void(std::size_t)> m_signalDeleteRequested;

    void clearRows();
};
