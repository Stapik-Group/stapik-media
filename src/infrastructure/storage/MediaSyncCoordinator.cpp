#include "MediaSyncCoordinator.hpp"
#include "Storage.hpp"

#include "stapik/cloud/CloudStorageException.hpp"

#include <glib.h>

Snapshot MediaSyncCoordinator::resolveOnConnect(const Snapshot& local, const CloudStorageClient& cloudClient)
{
    Snapshot cloud;

    try
    {
        const auto json = cloudClient.loadJson();
        if (json.empty())
        {
            cloudClient.saveJson(Storage::toJson(local));
            return local;
        }
        cloud = Storage::fromJson(json);
    }
    catch (const CloudStorageException&)
    {
        g_debug("Cloud unreachable right now — keep working with local data.");
        return local;
    }

    if (cloud.lastUpdate > local.lastUpdate)
    {
        Storage::save(cloud);
        return cloud;
    }

    if (local.lastUpdate > cloud.lastUpdate)
    {
        try { cloudClient.saveJson(Storage::toJson(local)); }
        catch (const CloudStorageException&)
        {
            g_debug("Cannot sync with cloud, will retry on next save.");
        }
    }

    return local;
}