#pragma once

#include "MediaEntry.hpp"
#include <optional>
#include <vector>

struct Snapshot
{
    std::vector<MediaEntry> entries;
    std::chrono::system_clock::time_point lastUpdate;
    std::optional<std::chrono::system_clock::time_point> lastKnownCloudUpdate;
};