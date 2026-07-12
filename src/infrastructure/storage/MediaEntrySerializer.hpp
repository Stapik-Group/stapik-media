#pragma once

#include "../../core/model/MediaEntry.hpp"
#include <nlohmann/json.hpp>

class MediaEntrySerializer
{
public:
    [[nodiscard]] static nlohmann::json toJson(const MediaEntry& entry);
    [[nodiscard]] static MediaEntry fromJson(const nlohmann::json& json);
};