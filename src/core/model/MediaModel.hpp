#pragma once

#include "Snapshot.hpp"

#include "stapik/cloud/CloudStorageClient.hpp"

#include <sigc++/signal.h>
#include <memory>
#include <vector>

class MediaModel
{
public:
    explicit MediaModel(Snapshot initial);
    [[nodiscard]] const std::vector<MediaEntry>& entries() const;

    void addEntry(MediaEntry entry);
    void updateEntry(std::size_t index, MediaEntry entry);
    void removeEntry(std::size_t index);

    void setCloudClient(std::unique_ptr<CloudStorageClient> client);
    void retrySync();

    sigc::signal<void()>& signalEntriesChanged();
private:
    Snapshot m_snapshot;
    std::unique_ptr<CloudStorageClient> m_cloudClient;
    sigc::signal<void()> m_signalEntriesChanged;

    void persist();
    void syncFromCloud();

    [[nodiscard]] bool isValidIndex(std::size_t index) const;
};
