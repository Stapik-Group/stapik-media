#include "MediaDocumentSchema.hpp"

#include "stapik/domain/IdGenerator.hpp"

#include <nlohmann/json.hpp>

namespace
{
    nlohmann::json legacyEntries(const nlohmann::json& legacy)
    {
        if (legacy.contains("payload") && legacy.at("payload").is_object() && legacy.at("payload").contains("entries"))
            return legacy.at("payload").at("entries");

        if (legacy.contains("entries"))
            return legacy.at("entries");

        return nlohmann::json::array();
    }

    void migrateFromLegacy(nlohmann::json& document)
    {
        nlohmann::json migrated = nlohmann::json::object();

        for (const auto* key : { "lastUpdate", "lastKnownCloudUpdate" })
        {
            if (document.contains(key))
                migrated[key] = document.at(key);
        }

        auto entries = legacyEntries(document);
        for (auto& entry : entries)
        {
            if (entry.is_object() && !entry.contains("id"))
                entry["id"] = stapik::domain::generateId();
        }

        migrated["entries"] = std::move(entries);
        document = std::move(migrated);
    }
}

stapik::document::SchemaMigrator createMediaSchemaMigrator()
{
    stapik::document::SchemaMigrator migrator(MEDIA_SCHEMA_VERSION);
    migrator.addStep(0, migrateFromLegacy);
    return migrator;
}
