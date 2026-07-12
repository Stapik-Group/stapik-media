#pragma once

#include "../../core/model/Snapshot.hpp"
#include "stapik/cloud/CloudStorageClient.hpp"

class MediaSyncCoordinator
{
public:
    [[nodiscard]] static Snapshot resolveOnConnect(const Snapshot& local, const CloudStorageClient& cloudClient);
};