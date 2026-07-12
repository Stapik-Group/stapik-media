#include "MediaListView.hpp"
#include "stapik/locale/LocaleManager.hpp"

MediaListView::MediaListView() : Box(Gtk::Orientation::VERTICAL, LIST_SPACING)
{
    m_emptyPlaceholder.set_text(LocaleManager::instance().translate("list.empty"));
    m_emptyPlaceholder.set_margin(LIST_SPACING * 4);
    m_emptyPlaceholder.add_css_class("dim-label");
}

void MediaListView::refresh(const std::vector<MediaEntry>& allEntries, const std::vector<std::size_t>& indicesToShow)
{
    clearRows();

    for (const auto index : indicesToShow)
    {
        if (index >= allEntries.size())
        {
            g_warning("MediaListView::refresh: index out of range: %zu", index);
            continue;
        }

        auto row = std::make_unique<MediaEntryRow>(allEntries[index]);
        row->signalEditRequested().connect([this, index] { m_signalEditRequested.emit(index); });
        row->signalDeleteRequested().connect([this, index] { m_signalDeleteRequested.emit(index); });
        append(*row);
        m_rows.push_back(std::move(row));
    }

    if (indicesToShow.empty())
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

void MediaListView::refreshLabels()
{
    for (const auto& row : m_rows)
        row->refreshLabels();
    m_emptyPlaceholder.set_text(LocaleManager::instance().translate("list.empty"));
}

sigc::signal<void(std::size_t)>& MediaListView::signalEditRequested()
{
    return m_signalEditRequested;
}

sigc::signal<void(std::size_t)>& MediaListView::signalDeleteRequested()
{
    return m_signalDeleteRequested;
}