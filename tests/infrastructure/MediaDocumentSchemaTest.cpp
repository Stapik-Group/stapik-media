#include "core/model/MediaDocument.hpp"
#include "infrastructure/storage/MediaDocumentSchema.hpp"
#include "infrastructure/storage/MediaEntrySerializer.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

#include <set>

namespace
{
    using stapik::document::MigrationStatus;

    nlohmann::json legacyEntry(const std::string& title)
    {
        auto json = MediaEntrySerializer::toJson(test::makeMovie(title, 2024, 3));
        json.erase("id");
        return json;
    }

    nlohmann::json legacyDocument(const nlohmann::json& entries)
    {
        return {
            {"lastUpdate", "2026-09-30T12:34:56Z"},
            {"lastKnownCloudUpdate", "2026-09-30T12:34:00Z"},
            {"payload", {{"entries", entries}}}
        };
    }
}

TEST(MediaDocumentSchemaTest, LegacyEnvelopeIsFlattened)
{
    auto document = legacyDocument(nlohmann::json::array({legacyEntry("Heat")}));

    EXPECT_EQ(createMediaSchemaMigrator().migrate(document, 0), MigrationStatus::Migrated);

    EXPECT_FALSE(document.contains("payload"));
    EXPECT_EQ(document.at("lastUpdate").get<std::string>(), "2026-09-30T12:34:56Z");
    EXPECT_EQ(document.at("lastKnownCloudUpdate").get<std::string>(), "2026-09-30T12:34:00Z");
    ASSERT_EQ(document.at("entries").size(), 1U);
}

TEST(MediaDocumentSchemaTest, EntriesReceiveUniqueIds)
{
    auto document = legacyDocument(nlohmann::json::array({legacyEntry("Heat"), legacyEntry("Ronin"), legacyEntry("Alien")}));

    EXPECT_EQ(createMediaSchemaMigrator().migrate(document, 0), MigrationStatus::Migrated);

    std::set<std::string> ids;
    for (const auto& entry : document.at("entries"))
    {
        const auto id = entry.at("id").get<std::string>();
        EXPECT_FALSE(id.empty());
        ids.insert(id);
    }
    EXPECT_EQ(ids.size(), 3U);
}

TEST(MediaDocumentSchemaTest, ExistingIdsArePreserved)
{
    auto entry = legacyEntry("Heat");
    entry["id"] = "keep-me";
    auto document = legacyDocument(nlohmann::json::array({entry}));

    EXPECT_EQ(createMediaSchemaMigrator().migrate(document, 0), MigrationStatus::Migrated);

    EXPECT_EQ(document.at("entries").at(0).at("id").get<std::string>(), "keep-me");
}

TEST(MediaDocumentSchemaTest, MigratedDocumentLoadsAsMediaDocument)
{
    auto document = legacyDocument(nlohmann::json::array({legacyEntry("Heat")}));
    EXPECT_EQ(createMediaSchemaMigrator().migrate(document, 0), MigrationStatus::Migrated);

    const auto restored = MediaDocument::fromJson(document);

    ASSERT_EQ(restored.entries.size(), 1U);
    EXPECT_EQ(restored.entries[0].title, "Heat");
    EXPECT_FALSE(restored.entries[0].id.empty());
    EXPECT_TRUE(restored.cloudBaseline.has_value());
}

TEST(MediaDocumentSchemaTest, LegacyDocumentWithoutEntriesBecomesEmpty)
{
    nlohmann::json document = {{"lastUpdate", "2026-09-30T12:34:56Z"}};

    EXPECT_EQ(createMediaSchemaMigrator().migrate(document, 0), MigrationStatus::Migrated);

    EXPECT_TRUE(document.at("entries").empty());
}

TEST(MediaDocumentSchemaTest, CurrentVersionIsLeftUntouched)
{
    MediaDocument source;
    source.entries = { test::makeMovie("Heat", 2024, 3) };
    auto document = source.toJson();
    const auto before = document;

    EXPECT_EQ(createMediaSchemaMigrator().migrate(document, MEDIA_SCHEMA_VERSION), MigrationStatus::UpToDate);

    EXPECT_EQ(document, before);
}

TEST(MediaDocumentSchemaTest, NewerVersionIsReported)
{
    nlohmann::json document = nlohmann::json::object();

    EXPECT_EQ(createMediaSchemaMigrator().migrate(document, MEDIA_SCHEMA_VERSION + 1), MigrationStatus::FromNewerVersion);
}
