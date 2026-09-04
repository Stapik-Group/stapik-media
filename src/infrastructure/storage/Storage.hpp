#pragma once

#include "../../core/model/Snapshot.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <optional>
#include <stdexcept>

class StorageException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class Storage
{
public:
    static void save(const Snapshot& snapshot);
    [[nodiscard]] static Snapshot load();

    [[nodiscard]] static nlohmann::json toJson(const Snapshot& snapshot);
    [[nodiscard]] static Snapshot fromJson(const nlohmann::json& json);

private:
    [[nodiscard]] static std::filesystem::path storagePath();
    static std::string serializeTimestamp(std::chrono::system_clock::time_point tp);
    static std::chrono::system_clock::time_point deserializeTimestamp(const std::string& str);
};
