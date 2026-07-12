#include "Storage.hpp"
#include "MediaEntrySerializer.hpp"

#include "stapik/sync/SyncEnvelope.hpp"
#include "stapik/storage/AppPaths.hpp"

#include <fstream>

nlohmann::json Storage::toJson(const Snapshot& snapshot)
{
    nlohmann::json entriesJson = nlohmann::json::array();
    for (const auto& entry : snapshot.entries)
        entriesJson.push_back(MediaEntrySerializer::toJson(entry));

    return stapik::sync::SyncEnvelope{
        snapshot.lastUpdate,
        { { "entries", entriesJson } }
    }.toJson();
}

Snapshot Storage::fromJson(const nlohmann::json& json)
{
    try
    {
        const auto [lastUpdate, payload] = stapik::sync::SyncEnvelope::fromJson(json);
        std::vector<MediaEntry> entries;
        for (const auto& e : payload.at("entries"))
            entries.push_back(MediaEntrySerializer::fromJson(e));

        return Snapshot{ std::move(entries), lastUpdate };
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