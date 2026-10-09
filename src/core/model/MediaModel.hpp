#pragma once

#include "MediaDocument.hpp"

#include "../../infrastructure/storage/MediaRepository.hpp"

#include "stapik/cloud/CloudStorageConfig.hpp"
#include "stapik/sync/CloudSession.hpp"
#include "stapik/sync/SyncStatus.hpp"

#include <sigc++/signal.h>
#include <optional>
#include <vector>

class MediaModel
{
public:
    MediaModel(MediaDocument initial, MediaRepository& repository);
    MediaModel(const MediaModel&) = delete;
    MediaModel& operator=(const MediaModel&) = delete;

    [[nodiscard]] const std::vector<MediaEntry>& entries() const;

    void addEntry(MediaEntry entry);
    void updateEntry(std::size_t index, MediaEntry entry);
    void removeEntry(std::size_t index);

    bool connectCloud(const CloudStorageConfig& config);
    void syncNow();
    void flush();

    [[nodiscard]] std::optional<CloudStorageConfig> cloudConfig() const;
    [[nodiscard]] bool isCloudConnected() const;
    [[nodiscard]] bool isSyncing() const;
    [[nodiscard]] stapik::sync::SyncStatus syncStatus() const;

    sigc::signal<void()>& signalEntriesChanged();
    sigc::signal<void(stapik::sync::SyncStatus)>& signalSyncStatusChanged();
private:
    static constexpr auto CLOUD_DOCUMENT_NAME = "media.json";

    MediaDocument m_document;
    MediaRepository& m_repository;
    stapik::sync::CloudSession<MediaDocument> m_session;
    sigc::signal<void()> m_signalEntriesChanged;
    sigc::signal<void(stapik::sync::SyncStatus)> m_signalSyncStatusChanged;

    void persist();
    void onDocumentReplaced(const MediaDocument& document);
    void onBaselineChanged(std::chrono::system_clock::time_point baseline);
    void onSyncOutcome(const stapik::sync::SyncOutcome<MediaDocument>& outcome);

    [[nodiscard]] bool isValidIndex(std::size_t index) const;
};
