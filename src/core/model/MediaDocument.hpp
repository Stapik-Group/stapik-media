#pragma once

#include "MediaEntry.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <optional>
#include <vector>

struct MediaDocument
{
    std::vector<MediaEntry> entries;
    std::vector<nlohmann::json> unreadableEntries;
    std::chrono::system_clock::time_point updatedAt{};
    std::optional<std::chrono::system_clock::time_point> cloudBaseline;

    [[nodiscard]] nlohmann::json toJson() const;
    [[nodiscard]] static MediaDocument fromJson(const nlohmann::json& json);

    [[nodiscard]] std::chrono::system_clock::time_point lastUpdate() const;
    [[nodiscard]] std::optional<std::chrono::system_clock::time_point> lastKnownCloudUpdate() const;
    [[nodiscard]] MediaDocument withLastKnownCloudUpdate(std::chrono::system_clock::time_point cloudUpdate) const;
};
