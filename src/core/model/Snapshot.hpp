#pragma once

#include "MediaEntry.hpp"
#include <vector>

struct Snapshot
{
    std::vector<MediaEntry> entries;
    std::chrono::system_clock::time_point lastUpdate;
};