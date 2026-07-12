#include "MediaListView.hpp"

#include "../../core/model/MediaEntry.hpp"
#include "stapik/locale/LocaleManager.hpp"

MediaListView::MediaListView() : Box(Gtk::Orientation::VERTICAL, LIST_SPACING)
{
    m_emptyPlaceholder.set_text(LocaleManager::instance().translate("list.empty"));
    m_emptyPlaceholder.set_margin(LIST_SPACING * 4);
    m_emptyPlaceholder.add_css_class("dim-label");
}

void MediaListView::refresh(const std::vector<MediaEntry>& allEntries, const MediaCategory filterCategory)
{
    clearRows();

    bool anyVisible = false;
    for (std::size_t i = 0; i < allEntries.size(); ++i)
    {
        if (allEntries[i].category != filterCategory)
            continue;

        anyVisible = true;
        auto row = std::make_unique<MediaEntryRow>(allEntries[i]);
        row->signalEditRequested().connect([this, i] { m_signalEditRequested.emit(i); });
        row->signalDeleteRequested().connect([this, i] { m_signalDeleteRequested.emit(i); });
        append(*row);
        m_rows.push_back(std::move(row));
    }

    if (!anyVisible)
        append(m_emptyPlaceholder);
}

void MediaListView::clearRows()
{
    for (const auto& row : m_rows)
        remove(*row);
    m_rows.clear();

    if (m_emptyPlaceholder.get_parent() != nullptr)
        remove(m_emptyPlaceholder);
}

void MediaListView::refreshLabels() const
{
    for (const auto& row : m_rows)
        row->refreshLabels();
}

sigc::signal<void(std::size_t)>& MediaListView::signalEditRequested()
{
    return m_signalEditRequested;
}

sigc::signal<void(std::size_t)>& MediaListView::signalDeleteRequested()
{
    return m_signalDeleteRequested;
}