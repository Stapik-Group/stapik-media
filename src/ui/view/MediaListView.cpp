#include "MediaListView.hpp"
#include "stapik/log/Log.hpp"

MediaListView::MediaListView() :
    Box(Gtk::Orientation::VERTICAL, LIST_SPACING),
    m_emptyState(EmptyState::DEFAULT_ICON, "list.empty")
{}

void MediaListView::refresh(const std::vector<MediaEntry>& allEntries, const std::vector<std::size_t>& indicesToShow)
{
    clearRows();

    for (const auto index : indicesToShow)
    {
        if (index >= allEntries.size())
        {
            stapik::log::warning("MediaListView::refresh: index out of range: {}", index);
            continue;
        }

        auto row = std::make_unique<MediaEntryRow>(allEntries[index]);
        row->setEditable(m_editable);
        row->signalEditRequested().connect([this, index] { m_signalEditRequested.emit(index); });
        row->signalDeleteRequested().connect([this, index] { m_signalDeleteRequested.emit(index); });
        append(*row);
        m_rows.push_back(std::move(row));
    }

    if (indicesToShow.empty())
        append(m_emptyState);
}

void MediaListView::setEditable(const bool editable)
{
    m_editable = editable;
    for (const auto& row : m_rows)
        row->setEditable(editable);
}

void MediaListView::clearRows()
{
    for (const auto& row : m_rows)
        remove(*row);
    m_rows.clear();

    if (m_emptyState.get_parent() != nullptr)
        remove(m_emptyState);
}

void MediaListView::refreshLabels()
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