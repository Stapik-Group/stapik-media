#pragma once

#include "MediaDocument.hpp"

#include "../../infrastructure/storage/MediaRepository.hpp"

#include "stapik/cloud/ICloudStorage.hpp"
#include "stapik/sync/SyncOutcome.hpp"

#include <sigc++/signal.h>
#include <memory>
#include <vector>

class MediaModel
{
public:
    MediaModel(MediaDocument initial, MediaRepository& repository);
    [[nodiscard]] const std::vector<MediaEntry>& entries() const;

    void addEntry(MediaEntry entry);
    void updateEntry(std::size_t index, MediaEntry entry);
    void removeEntry(std::size_t index);

    void setCloudClient(std::unique_ptr<ICloudStorage> storage);
    void retrySync();

    sigc::signal<void()>& signalEntriesChanged();
private:
    MediaDocument m_document;
    MediaRepository& m_repository;
    std::unique_ptr<ICloudStorage> m_cloudStorage;
    sigc::signal<void()> m_signalEntriesChanged;

    void persist();
    void syncFromCloud();
    void applyOutcome(const stapik::sync::SyncOutcome<MediaDocument>& outcome);

    [[nodiscard]] bool isValidIndex(std::size_t index) const;
};
