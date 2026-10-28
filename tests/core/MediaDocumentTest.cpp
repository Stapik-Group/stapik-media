#include "core/model/MediaDocument.hpp"

#include "infrastructure/storage/MediaEntrySerializer.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <tuple>
#include <vector>

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

namespace
{
    nlohmann::json documentWithEntries(const nlohmann::json& entries)
    {
        return { { "lastUpdate", "2026-09-30T12:34:56Z" }, { "entries", entries } };
    }

    nlohmann::json readableEntry(const std::string& title)
    {
        return MediaEntrySerializer::toJson(test::makeMovie(title, 2024, 3, title));
    }

    nlohmann::json withChange(nlohmann::json entry, const std::string& pointer, const nlohmann::json& value)
    {
        entry[nlohmann::json::json_pointer(pointer)] = value;
        return entry;
    }
}

TEST(MediaDocumentTest, UnreadableEntryDoesNotSpoilTheOthers)
{
    const auto unknownCategory = withChange(readableEntry("Future"), "/category", "podcast");

    const auto document = MediaDocument::fromJson(documentWithEntries({ readableEntry("Heat"), unknownCategory, readableEntry("Ronin") }));

    ASSERT_EQ(document.entries.size(), 2U);
    EXPECT_EQ(document.entries[0].title, "Heat");
    EXPECT_EQ(document.entries[1].title, "Ronin");
    ASSERT_EQ(document.unreadableEntries.size(), 1U);
    EXPECT_EQ(document.unreadableEntries[0], unknownCategory);
}

TEST(MediaDocumentTest, EveryKindOfBrokenEntryIsKeptAside)
{
    const std::vector<nlohmann::json> broken = {
        withChange(readableEntry("a"), "/category", "podcast"),
        withChange(readableEntry("b"), "/releaseDate/precision", "decade"),
        withChange(readableEntry("c"), "/releaseDate/date", "2024-02-30"),
        withChange(readableEntry("d"), "/releaseDate/date", "soon"),
        withChange(readableEntry("e"), "/consumed/month", 13),
        withChange(readableEntry("f"), "/details", nlohmann::json::object()),
        withChange(readableEntry("g"), "/title", 5),
        nlohmann::json(nullptr),
        nlohmann::json("text")
    };

    const auto document = MediaDocument::fromJson(documentWithEntries(nlohmann::json(broken)));

    EXPECT_TRUE(document.entries.empty());
    EXPECT_EQ(document.unreadableEntries, broken);
}

TEST(MediaDocumentTest, UnreadableEntriesAreWrittenBackUnchanged)
{
    const auto unknownCategory = withChange(readableEntry("Future"), "/category", "podcast");
    const auto document = MediaDocument::fromJson(documentWithEntries({ readableEntry("Heat"), unknownCategory }));

    const auto written = document.toJson().at("entries");

    ASSERT_EQ(written.size(), 2U);
    EXPECT_EQ(written[0], readableEntry("Heat"));
    EXPECT_EQ(written[1], unknownCategory);
}

TEST(MediaDocumentTest, UnreadableEntriesSurviveAnotherRoundTrip)
{
    const auto unknownCategory = withChange(readableEntry("Future"), "/category", "podcast");
    const auto first = MediaDocument::fromJson(documentWithEntries({ readableEntry("Heat"), unknownCategory }));

    const auto second = MediaDocument::fromJson(first.toJson());

    EXPECT_EQ(second.entries.size(), 1U);
    EXPECT_EQ(second.unreadableEntries, first.unreadableEntries);
}

TEST(MediaDocumentTest, EntriesThatAreNotAListAreRejected)
{
    EXPECT_THROW(std::ignore = MediaDocument::fromJson(documentWithEntries(nlohmann::json::object())), std::invalid_argument);
}
