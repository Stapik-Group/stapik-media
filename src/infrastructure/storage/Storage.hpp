#pragma once

#include "../../core/model/Snapshot.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>

class Storage
{
public:
    static void save(const Snapshot& snapshot);
    [[nodiscard]] static Snapshot load();

    [[nodiscard]] static nlohmann::json toJson(const Snapshot& snapshot);
    [[nodiscard]] static Snapshot fromJson(const nlohmann::json& json);

private:
    [[nodiscard]] static std::filesystem::path storagePath();
};
