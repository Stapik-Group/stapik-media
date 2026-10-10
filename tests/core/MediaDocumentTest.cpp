#include "core/model/MediaDocument.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

#include <tuple>

namespace
{
    using std::chrono::duration_cast;
    using std::chrono::microseconds;
    using std::chrono::seconds;
    using std::chrono::system_clock;

    system_clock::time_point at(const long long secondsSinceEpoch, const long long micros = 0)
    {
        return system_clock::time_point{duration_cast<system_clock::duration>(seconds{secondsSinceEpoch} + microseconds{micros})};
    }
}

TEST(MediaDocumentTest, RoundTripsEntriesAndTimestamps)
{
    MediaDocument document;
    document.entries = { test::makeMovie("Heat", 2024, 3), test::makeBook("Dune", 2023, 11, "book-2") };
    document.updatedAt = at(1'790'000'000);
    document.cloudBaseline = at(1'789'999'000, 123'456);

    const auto restored = MediaDocument::fromJson(document.toJson());

    ASSERT_EQ(restored.entries.size(), 2U);
    EXPECT_EQ(restored.entries[0].title, "Heat");
    EXPECT_EQ(restored.entries[1].id, "book-2");
    EXPECT_EQ(restored.updatedAt, document.updatedAt);
    ASSERT_TRUE(restored.cloudBaseline.has_value());
    EXPECT_EQ(std::chrono::floor<microseconds>(*restored.cloudBaseline), std::chrono::floor<microseconds>(*document.cloudBaseline));
}

TEST(MediaDocumentTest, BaselineIsOmittedWhenAbsent)
{
    MediaDocument document;
    document.updatedAt = at(1'790'000'000);

    const auto json = document.toJson();

    EXPECT_TRUE(json.contains("lastUpdate"));
    EXPECT_FALSE(json.contains("lastKnownCloudUpdate"));
}

TEST(MediaDocumentTest, MissingLastUpdateFallsBackToEpoch)
{
    const auto document = MediaDocument::fromJson(nlohmann::json{{"entries", nlohmann::json::array()}});

    EXPECT_EQ(document.updatedAt, system_clock::time_point{});
    EXPECT_FALSE(document.cloudBaseline.has_value());
    EXPECT_TRUE(document.entries.empty());
}

TEST(MediaDocumentTest, MissingEntriesIsRejected)
{
    EXPECT_THROW(std::ignore = MediaDocument::fromJson(nlohmann::json::object()), nlohmann::json::exception);
}

TEST(MediaDocumentTest, WithLastKnownCloudUpdateKeepsContent)
{
    MediaDocument document;
    document.entries = { test::makeMovie("Heat", 2024, 3) };
    document.updatedAt = at(1'790'000'000);

    const auto updated = document.withLastKnownCloudUpdate(at(1'790'000'500));

    EXPECT_EQ(updated.entries.size(), 1U);
    EXPECT_EQ(updated.lastUpdate(), document.lastUpdate());
    ASSERT_TRUE(updated.lastKnownCloudUpdate().has_value());
    EXPECT_EQ(*updated.lastKnownCloudUpdate(), at(1'790'000'500));
    EXPECT_FALSE(document.lastKnownCloudUpdate().has_value());
}
