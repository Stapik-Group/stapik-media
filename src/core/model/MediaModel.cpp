#include "MediaModel.hpp"

#include "stapik/domain/IdGenerator.hpp"
#include "stapik/log/Log.hpp"

#include <chrono>
#include <tuple>

MediaModel::MediaModel(MediaDocument initial, MediaRepository& repository) :
    m_document(std::move(initial)),
    m_repository(repository),
    m_session(stapik::sync::defaultCloudSessionHooks(CLOUD_DOCUMENT_NAME))
{
    m_session.signalDocumentReplaced().connect(sigc::mem_fun(*this, &MediaModel::onDocumentReplaced));
    m_session.signalBaselineChanged().connect(sigc::mem_fun(*this, &MediaModel::onBaselineChanged));
    m_session.signalOutcome().connect(sigc::mem_fun(*this, &MediaModel::onSyncOutcome));
    m_session.signalStatusChanged().connect([this](const stapik::sync::SyncStatus status)
    {
        m_signalSyncStatusChanged.emit(status);
    });

    if (m_repository.isWritable())
        m_session.syncNow(m_document);
}

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

bool MediaModel::connectCloud(const CloudStorageConfig& config)
{
    if (!m_repository.isWritable())
        return false;

    return m_session.connectWith(config, m_document);
}

void MediaModel::syncNow()
{
    if (m_repository.isWritable())
        m_session.syncNow(m_document);
}

void MediaModel::flush()
{
    m_session.flush();
}

std::optional<CloudStorageConfig> MediaModel::cloudConfig() const
{
    return m_session.config();
}

bool MediaModel::isCloudConnected() const
{
    return m_repository.isWritable() && m_session.isConnected();
}

bool MediaModel::isSyncing() const
{
    return m_session.status() == stapik::sync::SyncStatus::Syncing;
}

stapik::sync::SyncStatus MediaModel::syncStatus() const
{
    return m_session.status();
}

sigc::signal<void()>& MediaModel::signalEntriesChanged()
{
    return m_signalEntriesChanged;
}

sigc::signal<void(stapik::sync::SyncStatus)>& MediaModel::signalSyncStatusChanged()
{
    return m_signalSyncStatusChanged;
}

bool MediaModel::isValidIndex(const std::size_t index) const
{
    return index < m_document.entries.size();
}

void MediaModel::persist()
{
    m_document.updatedAt = std::chrono::system_clock::now();
    std::ignore = m_repository.save(m_document);

    if (m_repository.isWritable())
        m_session.pushChange(m_document);
}

void MediaModel::onDocumentReplaced(const MediaDocument& document)
{
    m_document = document;
    std::ignore = m_repository.save(m_document);
    m_signalEntriesChanged.emit();
}

void MediaModel::onBaselineChanged(const std::chrono::system_clock::time_point baseline)
{
    m_document.cloudBaseline = baseline;
    std::ignore = m_repository.save(m_document);
}

void MediaModel::onSyncOutcome(const stapik::sync::SyncOutcome<MediaDocument>& outcome)
{
    if (outcome.message.empty())
        return;

    if (outcome.state == stapik::sync::SyncState::RemoteUnreadable)
        stapik::log::warning("[Cloud] {}", outcome.message);
    else
        stapik::log::debug("[Cloud] {}", outcome.message);
}
