#include "MediaSyncCoordinator.hpp"
#include "Storage.hpp"

#include "stapik/cloud/CloudStorageException.hpp"

#include <glib.h>

Snapshot MediaSyncCoordinator::resolveOnConnect(const Snapshot &local, CloudStorageClient &cloudClient)
{
    std::optional<CloudDocument> remote;
    try
    {
        remote = cloudClient.loadDocument();
    } catch (const CloudStorageException &)
    {
        g_debug("Cloud unreachable right now — keep working with local data.");
        return local;
    }

    if (!remote.has_value())
        return pushWithConflictResolution(local, cloudClient, std::nullopt);

    if (remote->updatedAt > local.lastUpdate)
        return fromCloudDocument(remote.value());

    return pushWithConflictResolution(local, cloudClient, remote->updatedAt);
}

Snapshot MediaSyncCoordinator::pushLocalChange(const Snapshot &local, CloudStorageClient &cloudClient)
{
    return pushWithConflictResolution(local, cloudClient, local.lastKnownCloudUpdate);
}

Snapshot MediaSyncCoordinator::pushWithConflictResolution(const Snapshot &local, CloudStorageClient &cloudClient,
                                                          std::optional<std::chrono::system_clock::time_point> baseline)
{
    CloudWriteResult result;
    try
    {
        result = cloudClient.saveDocument(Storage::toJson(local),
                                          baseline.value_or(std::chrono::system_clock::time_point{}));
    } catch (const CloudStorageException &)
    {
        g_debug("Cannot sync with cloud, will retry on next save.");
        return local;
    }

    if (!result.conflict)
    {
        auto snapshot = local;
        snapshot.lastKnownCloudUpdate = result.document.updatedAt;
        return snapshot;
    }

    if (result.document.updatedAt > local.lastUpdate)
        return fromCloudDocument(result.document);

    try
    {
        const auto [document, conflict] = cloudClient.saveDocument(Storage::toJson(local), result.document.updatedAt);
        if (!conflict)
        {
            auto snapshot = local;
            snapshot.lastKnownCloudUpdate = document.updatedAt;
            return snapshot;
        }

        return fromCloudDocument(document);
    } catch (const CloudStorageException &)
    {
        g_debug("Cannot sync with cloud, will retry on next save.");
        return local;
    }
}

Snapshot MediaSyncCoordinator::fromCloudDocument(const CloudDocument &document)
{
    auto snapshot = Storage::fromJson(document.content);
    snapshot.lastKnownCloudUpdate = document.updatedAt;
    return snapshot;
}
