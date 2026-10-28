#pragma once

#include "MediaDocument.hpp"

#include "../../infrastructure/storage/MediaRepository.hpp"

#include "stapik/cloud/CloudStorageConfig.hpp"
#include "stapik/command/UndoStack.hpp"
#include "stapik/document/BackupInfo.hpp"
#include "stapik/storage/FileError.hpp"
#include "stapik/sync/CloudSession.hpp"
#include "stapik/sync/CloudSessionHooks.hpp"
#include "stapik/sync/SyncStatus.hpp"
#include "stapik/time/Clock.hpp"

#include <sigc++/signal.h>
#include <chrono>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

class MediaModel
{
public:
    using TimePoint = std::chrono::system_clock::time_point;
    using CloudOutcome = stapik::sync::SyncOutcome<MediaDocument>;

    static constexpr std::chrono::milliseconds CLOUD_FLUSH_TIMEOUT{ 5000 };

    MediaModel(MediaDocument initial, MediaRepository& repository);
    MediaModel(
        MediaDocument initial,
        MediaRepository& repository,
        stapik::sync::CloudSessionHooks cloudHooks,
        const stapik::sync::AsyncSyncOptions& syncOptions = {},
        const stapik::time::IClock& clock = stapik::time::systemClock());
    MediaModel(const MediaModel&) = delete;
    MediaModel& operator=(const MediaModel&) = delete;

    [[nodiscard]] const std::vector<MediaEntry>& entries() const;
    [[nodiscard]] std::optional<MediaEntry> entryById(const std::string& id) const;
    [[nodiscard]] std::set<unsigned> availableYears() const;

    bool addEntry(MediaEntry entry);
    bool updateEntry(const std::string& id, MediaEntry entry);
    void removeEntry(const std::string& id);

    void insertEntry(MediaEntry entry, std::optional<std::size_t> position = std::nullopt);
    bool replaceEntry(MediaEntry entry);
    std::optional<std::size_t> eraseEntry(const std::string& id);

    void requestYear(int year);

    [[nodiscard]] stapik::command::UndoStack& undoStack();

    bool connectCloud(const CloudStorageConfig& config);
    void syncNow();
    void disconnectCloud();
    void flushCloud(std::chrono::milliseconds timeout = CLOUD_FLUSH_TIMEOUT);

    [[nodiscard]] std::vector<stapik::document::BackupInfo> listBackups() const;
    bool restoreBackup(const stapik::document::BackupInfo& backup);

    [[nodiscard]] std::optional<CloudStorageConfig> cloudConfig() const;
    [[nodiscard]] bool isCloudConnected() const;
    [[nodiscard]] bool isSyncing() const;
    [[nodiscard]] stapik::sync::SyncStatus syncStatus() const;

    sigc::signal<void()>& signalEntriesChanged();
    sigc::signal<void()>& signalAvailableYearsChanged();
    sigc::signal<void(stapik::sync::SyncStatus)>& signalSyncStatusChanged();
    sigc::signal<void(const std::string&)>& signalSaveFailed();
    sigc::signal<void(const CloudOutcome&)>& signalCloudOutcome();
    sigc::signal<void()>& signalCloudConflict();
    sigc::signal<void(TimePoint)>& signalCloudApiKeyExpiring();
private:
    static constexpr auto CLOUD_DOCUMENT_NAME = "media.json";

    struct Location
    {
        std::optional<int> archiveYear;
        std::size_t index;
    };

    struct Touched
    {
        bool main = false;
        std::set<int> archives;
    };

    MediaDocument m_main;
    std::map<int, MediaDocument> m_archives;
    std::vector<MediaEntry> m_view;
    std::set<int> m_remoteYears;
    bool m_remoteYearsKnown = false;
    std::set<std::string> m_syncingPartitions;
    MediaRepository& m_repository;
    const stapik::time::IClock& m_clock;
    bool m_saveFailureReported = false;
    stapik::sync::CloudSession<MediaDocument> m_session;
    stapik::command::UndoStack m_undoStack;
    sigc::signal<void()> m_signalEntriesChanged;
    sigc::signal<void()> m_signalAvailableYearsChanged;
    sigc::signal<void(stapik::sync::SyncStatus)> m_signalSyncStatusChanged;
    sigc::signal<void(const std::string&)> m_signalSaveFailed;
    sigc::signal<void(const CloudOutcome&)> m_signalCloudOutcome;
    sigc::signal<void()> m_signalCloudConflict;
    sigc::signal<void(TimePoint)> m_signalCloudApiKeyExpiring;

    [[nodiscard]] int currentYear() const;
    [[nodiscard]] std::optional<int> archiveYearOf(const MediaEntry& entry) const;
    [[nodiscard]] static std::optional<std::size_t> indexIn(const MediaDocument& document, const std::string& id);
    [[nodiscard]] std::optional<Location> locate(const std::string& id) const;
    [[nodiscard]] MediaDocument& documentFor(const std::optional<int>& archiveYear);
    [[nodiscard]] bool canWriteYear(int year) const;

    [[nodiscard]] Touched normalize();
    void touch(const Touched& touched);
    void commit(const Touched& touched);
    void rebuildView();
    void saveMain();
    void saveArchive(int year, const MediaDocument& document);
    void reportSave(const stapik::storage::FileResult& result);

    void startCloudSync();
    void reconcileArchives();

    void onDocumentReplaced(const MediaDocument& document);
    void onBaselineChanged(std::chrono::system_clock::time_point baseline);
    void onSyncOutcome(const stapik::sync::SyncOutcome<MediaDocument>& outcome);
    void onPartitionReplaced(const std::string& partition, const MediaDocument& document);
    void onPartitionBaselineChanged(const std::string& partition, std::chrono::system_clock::time_point baseline);
    void onPartitionStatusChanged(const std::string& partition, stapik::sync::SyncStatus status);
    void onPartitionOutcome(const std::string& partition, const stapik::sync::SyncOutcome<MediaDocument>& outcome);
    void onAvailableYears(const std::optional<std::vector<int>>& years);
};
