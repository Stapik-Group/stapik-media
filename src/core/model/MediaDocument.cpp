#include "MediaDocument.hpp"

#include "../../infrastructure/storage/MediaEntrySerializer.hpp"

#include "stapik/sync/Timestamp.hpp"

nlohmann::json MediaDocument::toJson() const
{
    auto entriesJson = nlohmann::json::array();
    for (const auto& entry : entries)
        entriesJson.push_back(MediaEntrySerializer::toJson(entry));

    nlohmann::json json = {
        { "lastUpdate", stapik::sync::toIso8601(updatedAt) },
        { "entries", std::move(entriesJson) }
    };

    if (cloudBaseline.has_value())
        json["lastKnownCloudUpdate"] = stapik::sync::toIso8601(*cloudBaseline, stapik::sync::TimestampPrecision::Microseconds);

    return json;
}

MediaDocument MediaDocument::fromJson(const nlohmann::json& json)
{
    MediaDocument document;

    if (json.contains("lastUpdate") && json.at("lastUpdate").is_string())
        document.updatedAt = stapik::sync::fromIso8601(json.at("lastUpdate").get<std::string>());

    if (json.contains("lastKnownCloudUpdate") && json.at("lastKnownCloudUpdate").is_string())
        document.cloudBaseline = stapik::sync::parseIso8601(json.at("lastKnownCloudUpdate").get<std::string>());

    for (const auto& entryJson : json.at("entries"))
        document.entries.push_back(MediaEntrySerializer::fromJson(entryJson));

    return document;
}

std::chrono::system_clock::time_point MediaDocument::lastUpdate() const
{
    return updatedAt;
}

std::optional<std::chrono::system_clock::time_point> MediaDocument::lastKnownCloudUpdate() const
{
    return cloudBaseline;
}

MediaDocument MediaDocument::withLastKnownCloudUpdate(const std::chrono::system_clock::time_point cloudUpdate) const
{
    auto copy = *this;
    copy.cloudBaseline = cloudUpdate;
    return copy;
}
