#include "MediaModel.hpp"

#include "../command/EntryCommands.hpp"
#include "EntryIdentity.hpp"
#include "YearPartitioning.hpp"

#include "stapik/domain/IdGenerator.hpp"
#include "stapik/domain/YearMonth.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/sync/PartitionKey.hpp"
#include "stapik/sync/Timestamp.hpp"

#include <algorithm>
#include <chrono>
#include <format>
#include <memory>
#include <utility>

namespace
{
    bool isConflict(const MediaModel::CloudOutcome& outcome)
    {
        return outcome.state == stapik::sync::SyncState::LostRaceAcceptedServer || outcome.discardedLocalChange;
    }

    const char* stateName(const stapik::sync::SyncState state)
    {
        switch (state)
        {
            using enum stapik::sync::SyncState;
            case Synchronized: return "synchronized";
            case OfflineKeptLocal: return "offline, local version kept";
            case ServerWon: return "cloud version is newer";
            case LostRaceAcceptedServer: return "conflict, cloud version accepted";
            case LostRaceKeptLocal: return "conflict, local version kept";
            case RemoteUnreadable: return "cloud document unreadable";
            case AuthRejected: return "API key rejected";
            case DecryptionFailed: return "decryption failed";
            case DocumentTooLarge: return "document too large";
        }
        return "unknown";
    }

    void logOutcome(const std::string& subject, const MediaModel::CloudOutcome& outcome)
    {
        const auto details = outcome.message.empty() ? std::string{} : std::format(" ({})", outcome.message);

        switch (outcome.state)
        {
            using enum stapik::sync::SyncState;
            case RemoteUnreadable:
            case AuthRejected:
            case DecryptionFailed:
            case DocumentTooLarge:
                stapik::log::warning("[Cloud] {}: {}{}", subject, stateName(outcome.state), details);
                break;
            case Synchronized:
            case OfflineKeptLocal:
            case ServerWon:
            case LostRaceAcceptedServer:
            case LostRaceKeptLocal:
                stapik::log::info("[Cloud] {}: {}{}", subject, stateName(outcome.state), details);
                break;
        }
    }
}

MediaModel::MediaModel(MediaDocument initial, MediaRepository& repository) :
    MediaModel(std::move(initial), repository, stapik::sync::defaultCloudSessionHooks(CLOUD_DOCUMENT_NAME))
{}

MediaModel::MediaModel(
    MediaDocument initial,
    MediaRepository& repository,
    stapik::sync::CloudSessionHooks cloudHooks,
    const stapik::sync::AsyncSyncOptions& syncOptions,
    const stapik::time::IClock& clock) :
    m_main(std::move(initial)),
    m_repository(repository),
    m_clock(clock),
    m_session(std::move(cloudHooks), syncOptions)
{
    m_archives = m_repository.loadArchives();
    touch(normalize());
    m_activeYear = currentYear();
    rebuildView();

    m_session.signalDocumentReplaced().connect(sigc::mem_fun(*this, &MediaModel::onDocumentReplaced));
    m_session.signalBaselineChanged().connect(sigc::mem_fun(*this, &MediaModel::onBaselineChanged));
    m_session.signalOutcome().connect(sigc::mem_fun(*this, &MediaModel::onSyncOutcome));
    m_session.signalStatusChanged().connect([this](const stapik::sync::SyncStatus status)
    {
        m_signalSyncStatusChanged.emit(status);
    });
    m_session.signalPartitionReplaced().connect(sigc::mem_fun(*this, &MediaModel::onPartitionReplaced));
    m_session.signalPartitionBaselineChanged().connect(sigc::mem_fun(*this, &MediaModel::onPartitionBaselineChanged));
    m_session.signalPartitionStatusChanged().connect(sigc::mem_fun(*this, &MediaModel::onPartitionStatusChanged));
    m_session.signalPartitionOutcome().connect(sigc::mem_fun(*this, &MediaModel::onPartitionOutcome));
    m_session.signalAvailableYears().connect(sigc::mem_fun(*this, &MediaModel::onAvailableYears));
    m_session.signalApiKeyExpiring().connect([this](const TimePoint expiresAt) { m_signalCloudApiKeyExpiring.emit(expiresAt); });

    startCloudSync();
}

const std::vector<MediaEntry>& MediaModel::entries() const
{
    return m_view;
}

std::optional<MediaEntry> MediaModel::entryById(const std::string& id) const
{
    const auto location = locate(id);
    if (!location.has_value())
        return std::nullopt;

    const auto& document = location->archiveYear.has_value() ? m_archives.at(*location->archiveYear) : m_main;
    return document.entries.at(location->index);
}

std::set<unsigned> MediaModel::availableYears() const
{
    std::set<unsigned> years;
    for (const auto& entry : m_view)
        years.insert(static_cast<unsigned>(entry.consumed.year()));

    for (const auto year : m_remoteYears)
        years.insert(static_cast<unsigned>(year));

    return years;
}

std::size_t MediaModel::unreadableEntryCount() const
{
    auto count = m_main.unreadableEntries.size();
    for (const auto& archive : m_archives)
        count += archive.second.unreadableEntries.size();

    return count;
}

MediaModel::EntryResult MediaModel::addEntry(MediaEntry entry)
{
    if (!isWritable())
        return stapik::Failure(EntryError::ReadOnly);

    archiveClosedYears();

    if (!canWriteYear(entry.consumed.year()))
        return stapik::Failure(EntryError::ArchiveNotLoaded);

    if (hasConflictingWork(entry))
        return stapik::Failure(EntryError::Duplicate);

    if (entry.id.empty())
        entry.id = stapik::domain::generateId();

    m_undoStack.execute(std::make_unique<AddEntryCommand>(*this, std::move(entry)));
    return {};
}

MediaModel::EntryResult MediaModel::updateEntry(const std::string& id, MediaEntry entry)
{
    if (!isWritable())
        return stapik::Failure(EntryError::ReadOnly);

    archiveClosedYears();

    auto before = entryById(id);
    if (!before.has_value())
    {
        stapik::log::warning("MediaModel::updateEntry: unknown id {}", id);
        return stapik::Failure(EntryError::UnknownEntry);
    }

    if (!canWriteYear(entry.consumed.year()))
        return stapik::Failure(EntryError::ArchiveNotLoaded);

    entry.id = id;
    if (hasConflictingWork(entry))
        return stapik::Failure(EntryError::Duplicate);

    m_undoStack.execute(std::make_unique<UpdateEntryCommand>(*this, std::move(*before), std::move(entry)));
    return {};
}

void MediaModel::removeEntry(const std::string& id)
{
    if (!isWritable())
        return;

    archiveClosedYears();

    auto entry = entryById(id);
    if (!entry.has_value())
    {
        stapik::log::warning("MediaModel::removeEntry: unknown id {}", id);
        return;
    }

    m_undoStack.execute(std::make_unique<RemoveEntryCommand>(*this, std::move(*entry)));
}

void MediaModel::insertEntry(MediaEntry entry, const std::optional<std::size_t> position)
{
    const auto archiveYear = archiveYearOf(entry);
    auto& document = documentFor(archiveYear);

    const auto offset = std::min(position.value_or(document.entries.size()), document.entries.size());
    document.entries.insert(document.entries.begin() + static_cast<std::ptrdiff_t>(offset), std::move(entry));

    Touched touched;
    if (archiveYear.has_value())
        touched.archives.insert(*archiveYear);
    else
        touched.main = true;

    commit(touched);
}

bool MediaModel::replaceEntry(MediaEntry entry)
{
    const auto location = locate(entry.id);
    if (!location.has_value())
        return false;

    Touched touched;
    const auto newYear = archiveYearOf(entry);

    if (newYear == location->archiveYear)
    {
        documentFor(newYear).entries.at(location->index) = std::move(entry);
    }
    else
    {
        auto& oldDocument = documentFor(location->archiveYear);
        oldDocument.entries.erase(oldDocument.entries.begin() + static_cast<std::ptrdiff_t>(location->index));
        documentFor(newYear).entries.push_back(std::move(entry));

        if (location->archiveYear.has_value())
            touched.archives.insert(*location->archiveYear);
        else
            touched.main = true;
    }

    if (newYear.has_value())
        touched.archives.insert(*newYear);
    else
        touched.main = true;

    commit(touched);
    return true;
}

std::optional<std::size_t> MediaModel::eraseEntry(const std::string& id)
{
    const auto location = locate(id);
    if (!location.has_value())
        return std::nullopt;

    auto& document = documentFor(location->archiveYear);
    document.entries.erase(document.entries.begin() + static_cast<std::ptrdiff_t>(location->index));

    Touched touched;
    if (location->archiveYear.has_value())
        touched.archives.insert(*location->archiveYear);
    else
        touched.main = true;

    commit(touched);
    return location->index;
}

void MediaModel::requestYear(const int year)
{
    if (year >= currentYear() || m_archives.contains(year) || !isCloudConnected())
        return;

    m_session.loadPartition(stapik::sync::partitionKeyForYear(year), MediaDocument{});
}

stapik::command::UndoStack& MediaModel::undoStack()
{
    return m_undoStack;
}

bool MediaModel::connectCloud(const CloudStorageConfig& config)
{
    if (!m_repository.isWritable())
        return false;

    if (!m_session.connectWith(config, m_main))
        return false;

    reconcileArchives();
    m_session.fetchAvailableYears();
    return true;
}

void MediaModel::syncNow()
{
    startCloudSync();
}

void MediaModel::disconnectCloud()
{
    m_session.disconnect();

    m_remoteYears.clear();
    m_remoteYearsKnown = false;
    m_syncingPartitions.clear();

    if (m_main.cloudBaseline.has_value())
    {
        m_main.cloudBaseline.reset();
        saveMain();
    }

    for (auto& [year, archive] : m_archives)
    {
        if (!archive.cloudBaseline.has_value())
            continue;

        archive.cloudBaseline.reset();
        saveArchive(year, archive);
    }

    m_signalAvailableYearsChanged.emit();
}

void MediaModel::flushCloud(const std::chrono::milliseconds timeout)
{
    m_session.flushAndWait(timeout);
}

std::vector<stapik::document::BackupInfo> MediaModel::listBackups() const
{
    return m_repository.listBackups();
}

bool MediaModel::restoreBackup(const stapik::document::BackupInfo& backup)
{
    if (!m_repository.isWritable())
        return false;

    auto loaded = m_repository.loadBackup(backup);
    if (loaded.status != stapik::document::LoadStatus::Loaded)
        return false;

    const auto baseline = m_main.cloudBaseline;
    m_main = std::move(*loaded.document);
    m_main.cloudBaseline = baseline;
    m_undoStack.clear();

    auto touched = normalize();
    touched.main = true;
    commit(touched);
    return true;
}

std::optional<CloudStorageConfig> MediaModel::cloudConfig() const
{
    return m_session.config();
}

bool MediaModel::isWritable() const
{
    return m_repository.isWritable();
}

bool MediaModel::isCloudConnected() const
{
    return m_repository.isWritable() && m_session.isConnected();
}

bool MediaModel::hasCloudConnection() const
{
    return m_session.isConnected();
}

bool MediaModel::isSyncing() const
{
    return m_session.status() == stapik::sync::SyncStatus::Syncing || !m_syncingPartitions.empty();
}

stapik::sync::SyncStatus MediaModel::syncStatus() const
{
    return m_session.status();
}

sigc::signal<void()>& MediaModel::signalEntriesChanged()
{
    return m_signalEntriesChanged;
}

sigc::signal<void(const std::string&)>& MediaModel::signalSaveFailed()
{
    return m_signalSaveFailed;
}

sigc::signal<void(const MediaModel::CloudOutcome&)>& MediaModel::signalCloudOutcome()
{
    return m_signalCloudOutcome;
}

sigc::signal<void()>& MediaModel::signalCloudConflict()
{
    return m_signalCloudConflict;
}

sigc::signal<void(MediaModel::TimePoint)>& MediaModel::signalCloudApiKeyExpiring()
{
    return m_signalCloudApiKeyExpiring;
}

sigc::signal<void()>& MediaModel::signalAvailableYearsChanged()
{
    return m_signalAvailableYearsChanged;
}

sigc::signal<void(stapik::sync::SyncStatus)>& MediaModel::signalSyncStatusChanged()
{
    return m_signalSyncStatusChanged;
}

int MediaModel::currentYear() const
{
    return stapik::domain::YearMonth::current(m_clock).year();
}

std::optional<int> MediaModel::archiveYearOf(const MediaEntry& entry) const
{
    return YearPartitioning::archiveYearOf(entry, currentYear());
}

std::optional<std::size_t> MediaModel::indexIn(const MediaDocument& document, const std::string& id)
{
    const auto found = std::ranges::find(document.entries, id, &MediaEntry::id);
    if (found == document.entries.end())
        return std::nullopt;

    return static_cast<std::size_t>(found - document.entries.begin());
}

std::optional<MediaModel::Location> MediaModel::locate(const std::string& id) const
{
    if (const auto index = indexIn(m_main, id); index.has_value())
        return Location{ .archiveYear = std::nullopt, .index = *index };

    for (const auto& [year, document] : m_archives)
    {
        if (const auto index = indexIn(document, id); index.has_value())
            return Location{ .archiveYear = year, .index = *index };
    }

    return std::nullopt;
}

MediaDocument& MediaModel::documentFor(const std::optional<int>& archiveYear)
{
    return archiveYear.has_value() ? m_archives[*archiveYear] : m_main;
}

bool MediaModel::canWriteYear(const int year) const
{
    if (year >= currentYear() || m_archives.contains(year) || !isCloudConnected())
        return true;

    return m_remoteYearsKnown && !m_remoteYears.contains(year);
}

bool MediaModel::hasConflictingWork(const MediaEntry& candidate) const
{
    if (const auto existing = entryById(candidate.id); existing.has_value() && EntryIdentity::sameWork(*existing, candidate))
        return false;

    return std::ranges::any_of(m_view, [&candidate](const MediaEntry& entry)
    {
        return entry.id != candidate.id && EntryIdentity::sameWork(entry, candidate);
    });
}

MediaModel::Touched MediaModel::normalize()
{
    Touched touched;
    const auto year = currentYear();

    std::vector<MediaEntry> remaining;
    for (auto& entry : m_main.entries)
    {
        const auto entryYear = entry.consumed.year();
        if (entryYear >= year)
        {
            remaining.push_back(std::move(entry));
            continue;
        }

        const auto existed = m_archives.contains(entryYear);
        auto& archive = m_archives[entryYear];

        const auto duplicate = existed && YearPartitioning::containsSameContent(archive.entries, entry);
        if (duplicate)
            stapik::log::warning("Skipped an entry '{}' that is already in the archive of {}", entry.title, entryYear);

        if (!duplicate)
        {
            archive.entries.push_back(std::move(entry));
            touched.archives.insert(entryYear);
        }

        touched.main = true;
    }

    m_main.entries = std::move(remaining);

    return touched;
}

void MediaModel::archiveClosedYears()
{
    const auto year = currentYear();
    if (year == m_activeYear)
        return;

    m_activeYear = year;

    const auto touched = normalize();
    if (touched.main || !touched.archives.empty())
        commit(touched);
}

void MediaModel::touch(const Touched& touched)
{
    const auto now = m_clock.now();
    const auto writable = m_repository.isWritable();

    for (const auto year : touched.archives)
    {
        auto& archive = m_archives[year];
        archive.updatedAt = now;
        saveArchive(year, archive);

        if (writable)
        {
            stapik::log::debug("[Cloud] The archive of {} was saved and queued, updated {}", year, stapik::sync::toIso8601(archive.updatedAt));
            m_session.pushPartitionChange(stapik::sync::partitionKeyForYear(year), archive);
        }
    }

    if (touched.main)
    {
        m_main.updatedAt = now;
        saveMain();

        if (writable)
        {
            stapik::log::debug("[Cloud] The main document was saved and queued, updated {}", stapik::sync::toIso8601(m_main.updatedAt));
            m_session.pushChange(m_main);
        }
    }
}

void MediaModel::commit(const Touched& touched)
{
    touch(touched);
    rebuildView();
    m_signalEntriesChanged.emit();
}

void MediaModel::rebuildView()
{
    m_view.clear();

    for (const auto& [year, archive] : m_archives)
        m_view.insert(m_view.end(), archive.entries.begin(), archive.entries.end());

    m_view.insert(m_view.end(), m_main.entries.begin(), m_main.entries.end());
}

void MediaModel::saveMain()
{
    if (!m_repository.isWritable())
        return;

    reportSave(m_repository.save(m_main));
}

void MediaModel::saveArchive(const int year, const MediaDocument& document)
{
    if (!m_repository.isWritable())
        return;

    reportSave(m_repository.saveArchive(year, document));
}

void MediaModel::reportSave(const stapik::storage::FileResult& result)
{
    if (result)
    {
        m_saveFailureReported = false;
        return;
    }

    if (!std::exchange(m_saveFailureReported, true))
        m_signalSaveFailed.emit(result.error().message());
}

void MediaModel::startCloudSync()
{
    if (!isCloudConnected())
        return;

    m_session.syncNow(m_main);
    reconcileArchives();
    m_session.fetchAvailableYears();
}

void MediaModel::reconcileArchives()
{
    for (const auto& [year, archive] : m_archives)
        m_session.loadPartition(stapik::sync::partitionKeyForYear(year), archive);
}

void MediaModel::onDocumentReplaced(const MediaDocument& document)
{
    stapik::log::info("[Cloud] The main document was replaced by the cloud version: {} entries, updated {}",
        document.entries.size(), stapik::sync::toIso8601(document.lastUpdate()));

    m_main = document;
    saveMain();
    m_undoStack.clear();
    commit(normalize());
}

void MediaModel::onBaselineChanged(const std::chrono::system_clock::time_point baseline)
{
    m_main.cloudBaseline = baseline;
    saveMain();
}

void MediaModel::onSyncOutcome(const CloudOutcome& outcome)
{
    logOutcome("main document", outcome);
    m_signalCloudOutcome.emit(outcome);

    if (isConflict(outcome))
        m_signalCloudConflict.emit();
}

void MediaModel::onPartitionReplaced(const std::string& partition, const MediaDocument& document)
{
    const auto year = stapik::sync::yearFromPartitionKey(partition);
    if (!year.has_value())
        return;

    stapik::log::info("[Cloud] The archive of {} was replaced by the cloud version: {} entries, updated {}",
        *year, document.entries.size(), stapik::sync::toIso8601(document.lastUpdate()));

    m_archives[*year] = document;
    saveArchive(*year, document);
    m_undoStack.clear();
    rebuildView();
    m_signalEntriesChanged.emit();
}

void MediaModel::onPartitionBaselineChanged(const std::string& partition, const std::chrono::system_clock::time_point baseline)
{
    const auto year = stapik::sync::yearFromPartitionKey(partition);
    if (!year.has_value())
        return;

    const auto found = m_archives.find(*year);
    if (found == m_archives.end())
        return;

    found->second.cloudBaseline = baseline;
    saveArchive(*year, found->second);
}

void MediaModel::onPartitionStatusChanged(const std::string& partition, const stapik::sync::SyncStatus status)
{
    if (status == stapik::sync::SyncStatus::Syncing)
        m_syncingPartitions.insert(partition);
    else
        m_syncingPartitions.erase(partition);

    m_signalSyncStatusChanged.emit(m_session.status());
}

void MediaModel::onPartitionOutcome(const std::string& partition, const CloudOutcome& outcome)
{
    logOutcome(std::format("partition {}", partition), outcome);

    if (isConflict(outcome))
        m_signalCloudConflict.emit();
}

void MediaModel::onAvailableYears(const std::optional<std::vector<int>>& years)
{
    m_remoteYearsKnown = years.has_value();
    m_remoteYears.clear();

    if (years.has_value())
        m_remoteYears.insert(years->begin(), years->end());

    m_signalAvailableYearsChanged.emit();
}
