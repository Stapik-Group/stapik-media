#include "MediaModel.hpp"

#include "../../infrastructure/storage/Storage.hpp"
#include "../../infrastructure/storage/MediaSyncCoordinator.hpp"

#include <chrono>
#include <glib.h>

MediaModel::MediaModel(Snapshot initial) : m_snapshot(std::move(initial)) {}

const std::vector<MediaEntry>& MediaModel::entries() const
{
    return m_snapshot.entries;
}

void MediaModel::addEntry(MediaEntry entry)
{
    m_snapshot.entries.push_back(std::move(entry));
    persist();
    m_signalEntriesChanged.emit();
}

void MediaModel::updateEntry(const std::size_t index, MediaEntry entry)
{
    if (!isValidIndex(index))
    {
        g_warning("MediaModel::updateEntry: invalid index %zu", index);
        return;
    }

    m_snapshot.entries.at(index) = std::move(entry);
    persist();
    m_signalEntriesChanged.emit();
}

void MediaModel::removeEntry(const std::size_t index)
{
    if (!isValidIndex(index))
    {
        g_warning("MediaModel::removeEntry: invalid index %zu", index);
        return;
    }

    m_snapshot.entries.erase(m_snapshot.entries.begin() + static_cast<std::ptrdiff_t>(index));
    persist();
    m_signalEntriesChanged.emit();
}

void MediaModel::setCloudClient(std::unique_ptr<CloudStorageClient> client)
{
    m_cloudClient = std::move(client);
    syncFromCloud();
}

void MediaModel::retrySync()
{
    syncFromCloud();
}

sigc::signal<void()>& MediaModel::signalEntriesChanged()
{
    return m_signalEntriesChanged;
}

bool MediaModel::isValidIndex(const std::size_t index) const
{
    return index < m_snapshot.entries.size();
}

void MediaModel::persist()
{
    m_snapshot.lastUpdate = std::chrono::system_clock::now();

    if (m_cloudClient == nullptr)
        m_snapshot = MediaSyncCoordinator::pushLocalChange(m_snapshot, *m_cloudClient);

    Storage::save(m_snapshot);
}

void MediaModel::syncFromCloud()
{
    if (m_cloudClient == nullptr)
        return;

    m_snapshot = MediaSyncCoordinator::resolveOnConnect(m_snapshot, *m_cloudClient);
    Storage::save(m_snapshot);
    m_signalEntriesChanged.emit();
}