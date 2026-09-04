#include "Storage.hpp"
#include "MediaEntrySerializer.hpp"

#include "stapik/sync/SyncEnvelope.hpp"
#include "stapik/storage/AppPaths.hpp"

#include <fstream>
#include <chrono>
#include <ctime>


nlohmann::json Storage::toJson(const Snapshot& snapshot)
{
    nlohmann::json entriesJson = nlohmann::json::array();
    for (const auto& entry : snapshot.entries)
        entriesJson.push_back(MediaEntrySerializer::toJson(entry));

    auto json = stapik::sync::SyncEnvelope{
        snapshot.lastUpdate,
        { { "entries", entriesJson } }
    }.toJson();

    if (snapshot.lastKnownCloudUpdate.has_value())
        json["lastKnownCloudUpdate"] = serializeTimestamp(snapshot.lastKnownCloudUpdate.value());

    return json;
}

Snapshot Storage::fromJson(const nlohmann::json& json)
{
    try
    {
        const auto [lastUpdate, payload] = stapik::sync::SyncEnvelope::fromJson(json);
        std::vector<MediaEntry> entries;
        for (const auto& e : payload.at("entries"))
            entries.push_back(MediaEntrySerializer::fromJson(e));

        std::optional<std::chrono::system_clock::time_point> lastKnownCloudUpdate;
        if (json.contains("lastKnownCloudUpdate"))
            lastKnownCloudUpdate = deserializeTimestamp(json.at("lastKnownCloudUpdate").get<std::string>());

        return Snapshot{ std::move(entries), lastUpdate, lastKnownCloudUpdate };
    }
    catch (const nlohmann::json::exception&)
    {
        return Snapshot{};
    }
}

void Storage::save(const Snapshot& snapshot)
{
    const auto path = storagePath();
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path);
    file << toJson(snapshot).dump(2);
}

Snapshot Storage::load()
{
    const auto path = storagePath();
    if (!std::filesystem::exists(path))
        return Snapshot{};

    std::ifstream file(path);
    if (!file.is_open())
        return Snapshot{};

    try
    {
        return fromJson(nlohmann::json::parse(file));
    }
    catch (const nlohmann::json::exception&)
    {
        return Snapshot{};
    }
}

std::filesystem::path Storage::storagePath()
{
    return AppPaths::userDataDir("stapikmedia") / "media.json";
}

std::string Storage::serializeTimestamp(const std::chrono::system_clock::time_point tp)
{
    return std::format("{:%Y-%m-%dT%H:%M:%SZ}", std::chrono::floor<std::chrono::seconds>(tp));
}

std::chrono::system_clock::time_point Storage::deserializeTimestamp(const std::string& str)
{
    if (str.size() < 19)
        throw StorageException("Invalid ISO-8601 timestamp: " + str);

    std::tm tm{};
    tm.tm_year = std::stoi(str.substr(0, 4)) - 1900;
    tm.tm_mon = std::stoi(str.substr(5, 2)) - 1;
    tm.tm_mday = std::stoi(str.substr(8, 2));
    tm.tm_hour = std::stoi(str.substr(11, 2));
    tm.tm_min = std::stoi(str.substr(14, 2));
    tm.tm_sec = std::stoi(str.substr(17, 2));

    const auto time = timegm(&tm);
    return std::chrono::system_clock::from_time_t(time);
}