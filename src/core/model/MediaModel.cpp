#include "MediaModel.hpp"

#include "../command/EntryCommands.hpp"

#include "stapik/domain/IdGenerator.hpp"
#include "stapik/log/Log.hpp"

#include <algorithm>
#include <chrono>
#include <memory>
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

    m_undoStack.execute(std::make_unique<AddEntryCommand>(*this, std::move(entry)));
}

void MediaModel::updateEntry(const std::string& id, MediaEntry entry)
{
    const auto index = indexOf(id);
    if (!index.has_value())
    {
        stapik::log::warning("MediaModel::updateEntry: unknown id {}", id);
        return;
    }

    entry.id = id;
    m_undoStack.execute(std::make_unique<UpdateEntryCommand>(*this, m_document.entries.at(*index), std::move(entry)));
}

void MediaModel::removeEntry(const std::string& id)
{
    const auto index = indexOf(id);
    if (!index.has_value())
    {
        stapik::log::warning("MediaModel::removeEntry: unknown id {}", id);
        return;
    }

    m_undoStack.execute(std::make_unique<RemoveEntryCommand>(*this, m_document.entries.at(*index), *index));
}

void MediaModel::insertEntry(const std::size_t position, MediaEntry entry)
{
    const auto offset = static_cast<std::ptrdiff_t>(std::min(position, m_document.entries.size()));
    m_document.entries.insert(m_document.entries.begin() + offset, std::move(entry));
    persist();
    m_signalEntriesChanged.emit();
}

bool MediaModel::replaceEntry(MediaEntry entry)
{
    const auto index = indexOf(entry.id);
    if (!index.has_value())
        return false;

    m_document.entries.at(*index) = std::move(entry);
    persist();
    m_signalEntriesChanged.emit();
    return true;
}

bool MediaModel::eraseEntry(const std::string& id)
{
    const auto index = indexOf(id);
    if (!index.has_value())
        return false;

    m_document.entries.erase(m_document.entries.begin() + static_cast<std::ptrdiff_t>(*index));
    persist();
    m_signalEntriesChanged.emit();
    return true;
}

std::optional<std::size_t> MediaModel::indexOf(const std::string& id) const
{
    const auto found = std::ranges::find(m_document.entries, id, &MediaEntry::id);
    if (found == m_document.entries.end())
        return std::nullopt;

    return static_cast<std::size_t>(found - m_document.entries.begin());
}

stapik::command::UndoStack& MediaModel::undoStack()
{
    return m_undoStack;
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
    m_undoStack.clear();
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
