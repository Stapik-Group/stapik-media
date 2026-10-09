#include "MediaModel.hpp"

#include "stapik/domain/IdGenerator.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/sync/SyncCoordinator.hpp"

#include <chrono>
#include <tuple>

MediaModel::MediaModel(MediaDocument initial, MediaRepository& repository) :
    m_document(std::move(initial)),
    m_repository(repository)
{}

const std::vector<MediaEntry>& MediaModel::entries() const
{
    return m_document.entries;
}

void MediaModel::addEntry(MediaEntry entry)
{
    if (entry.id.empty())
        entry.id = stapik::domain::generateId();

    m_document.entries.push_back(std::move(entry));
    persist();
    m_signalEntriesChanged.emit();
}

void MediaModel::updateEntry(const std::size_t index, MediaEntry entry)
{
    if (!isValidIndex(index))
    {
        stapik::log::warning("MediaModel::updateEntry: invalid index {}", index);
        return;
    }

    if (entry.id.empty())
        entry.id = m_document.entries.at(index).id;

    m_document.entries.at(index) = std::move(entry);
    persist();
    m_signalEntriesChanged.emit();
}

void MediaModel::removeEntry(const std::size_t index)
{
    if (!isValidIndex(index))
    {
        stapik::log::warning("MediaModel::removeEntry: invalid index {}", index);
        return;
    }

    m_document.entries.erase(m_document.entries.begin() + static_cast<std::ptrdiff_t>(index));
    persist();
    m_signalEntriesChanged.emit();
}

void MediaModel::setCloudClient(std::unique_ptr<ICloudStorage> storage)
{
    m_cloudStorage = std::move(storage);
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
    return index < m_document.entries.size();
}

void MediaModel::persist()
{
    m_document.updatedAt = std::chrono::system_clock::now();

    if (m_cloudStorage != nullptr)
        applyOutcome(stapik::sync::SyncCoordinator<MediaDocument>(*m_cloudStorage).pushLocalChange(m_document));

    std::ignore = m_repository.save(m_document);
}

void MediaModel::syncFromCloud()
{
    if (m_cloudStorage == nullptr)
        return;

    applyOutcome(stapik::sync::SyncCoordinator<MediaDocument>(*m_cloudStorage).resolveOnConnect(m_document));
    std::ignore = m_repository.save(m_document);
    m_signalEntriesChanged.emit();
}

void MediaModel::applyOutcome(const stapik::sync::SyncOutcome<MediaDocument>& outcome)
{
    m_document = outcome.document;
}
