#include "MediaModel.hpp"

#include "../command/EntryCommands.hpp"
#include "YearPartitioning.hpp"

#include "stapik/domain/IdGenerator.hpp"
#include "stapik/domain/YearMonth.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/sync/PartitionKey.hpp"

#include <algorithm>
#include <chrono>
#include <memory>
#include <tuple>

MediaModel::MediaModel(MediaDocument initial, MediaRepository& repository) :
    m_main(std::move(initial)),
    m_repository(repository),
    m_session(stapik::sync::defaultCloudSessionHooks(CLOUD_DOCUMENT_NAME))
{
    m_archives = m_repository.loadArchives();
    touch(normalize());
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

bool MediaModel::addEntry(MediaEntry entry)
{
    if (!canWriteYear(entry.consumed.year()))
        return false;

    if (entry.id.empty())
        entry.id = stapik::domain::generateId();

    m_undoStack.execute(std::make_unique<AddEntryCommand>(*this, std::move(entry)));
    return true;
}

bool MediaModel::updateEntry(const std::string& id, MediaEntry entry)
{
    auto before = entryById(id);
    if (!before.has_value())
    {
        stapik::log::warning("MediaModel::updateEntry: unknown id {}", id);
        return false;
    }

    if (!canWriteYear(entry.consumed.year()))
        return false;

    entry.id = id;
    m_undoStack.execute(std::make_unique<UpdateEntryCommand>(*this, std::move(*before), std::move(entry)));
    return true;
}

void MediaModel::removeEntry(const std::string& id)
{
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

sigc::signal<void()>& MediaModel::signalAvailableYearsChanged()
{
    return m_signalAvailableYearsChanged;
}

sigc::signal<void(stapik::sync::SyncStatus)>& MediaModel::signalSyncStatusChanged()
{
    return m_signalSyncStatusChanged;
}

int MediaModel::currentYear()
{
    return stapik::domain::YearMonth::current().year();
}

std::optional<int> MediaModel::archiveYearOf(const MediaEntry& entry)
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

        if (!duplicate)
        {
            archive.entries.push_back(std::move(entry));
            touched.archives.insert(entryYear);
        }

        touched.main = true;
    }

    if (touched.main)
        m_main.entries = std::move(remaining);

    return touched;
}

void MediaModel::touch(const Touched& touched)
{
    const auto now = std::chrono::system_clock::now();
    const auto writable = m_repository.isWritable();

    for (const auto year : touched.archives)
    {
        auto& archive = m_archives[year];
        archive.updatedAt = now;
        std::ignore = m_repository.saveArchive(year, archive);

        if (writable)
            m_session.pushPartitionChange(stapik::sync::partitionKeyForYear(year), archive);
    }

    if (touched.main)
    {
        m_main.updatedAt = now;
        std::ignore = m_repository.save(m_main);

        if (writable)
            m_session.pushChange(m_main);
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
    m_main = document;
    std::ignore = m_repository.save(m_main);
    m_undoStack.clear();
    commit(normalize());
}

void MediaModel::onBaselineChanged(const std::chrono::system_clock::time_point baseline)
{
    m_main.cloudBaseline = baseline;
    std::ignore = m_repository.save(m_main);
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

void MediaModel::onPartitionReplaced(const std::string& partition, const MediaDocument& document)
{
    const auto year = stapik::sync::yearFromPartitionKey(partition);
    if (!year.has_value())
        return;

    m_archives[*year] = document;
    std::ignore = m_repository.saveArchive(*year, document);
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
    std::ignore = m_repository.saveArchive(*year, found->second);
}

void MediaModel::onPartitionStatusChanged(const std::string& partition, const stapik::sync::SyncStatus status)
{
    if (status == stapik::sync::SyncStatus::Syncing)
        m_syncingPartitions.insert(partition);
    else
        m_syncingPartitions.erase(partition);

    m_signalSyncStatusChanged.emit(m_session.status());
}

void MediaModel::onPartitionOutcome(const std::string& partition, const stapik::sync::SyncOutcome<MediaDocument>& outcome)
{
    if (outcome.message.empty())
        return;

    if (outcome.state == stapik::sync::SyncState::RemoteUnreadable)
        stapik::log::warning("[Cloud] Partition {}: {}", partition, outcome.message);
    else
        stapik::log::debug("[Cloud] Partition {}: {}", partition, outcome.message);
}

void MediaModel::onAvailableYears(const std::optional<std::vector<int>>& years)
{
    m_remoteYearsKnown = years.has_value();
    m_remoteYears.clear();

    if (years.has_value())
        m_remoteYears.insert(years->begin(), years->end());

    m_signalAvailableYearsChanged.emit();
}
