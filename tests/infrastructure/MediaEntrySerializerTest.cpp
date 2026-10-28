#include "infrastructure/storage/MediaEntrySerializer.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

#include <tuple>

#include <vector>

namespace
{
    std::vector<MediaEntry> oneEntryPerCategory()
    {
        using test::makeEntry;
        return {
            makeEntry(MediaCategory::Movie, "Heat", 2024, 3, ScreenDetails{"Mann", "Crime"}, "1"),
            makeEntry(MediaCategory::Series, "Dark", 2023, 11, ScreenDetails{"Odar", "Mystery"}, "2"),
            makeEntry(MediaCategory::Cartoon, "Up", 2022, 1, ScreenDetails{"Docter", "Adventure"}, "3"),
            makeEntry(MediaCategory::Book, "Dune", 2021, 7, BookDetails{"Herbert", "Sci-Fi", true}, "4"),
            makeEntry(MediaCategory::Album, "Kind of Blue", 2020, 12, AlbumDetails{"Miles Davis", "Columbia", "Jazz"}, "5"),
            makeEntry(MediaCategory::Game, "Celeste", 2019, 9, GameDetails{"Maddy Makes Games", "Maddy Makes Games", "PC"}, "6")
        };
    }
}

TEST(MediaEntrySerializerTest, EveryCategoryRoundTrips)
{
    for (const auto& entry : oneEntryPerCategory())
    {
        const auto json = MediaEntrySerializer::toJson(entry);
        const auto restored = MediaEntrySerializer::fromJson(json);

        EXPECT_EQ(MediaEntrySerializer::toJson(restored), json) << entry.title;
        EXPECT_EQ(restored.category, entry.category);
        EXPECT_EQ(restored.id, entry.id);
        EXPECT_EQ(restored.consumed, entry.consumed);
    }
}

TEST(MediaEntrySerializerTest, ConsumedDateKeepsItsFileFormat)
{
    const auto json = MediaEntrySerializer::toJson(test::makeMovie("Heat", 2024, 3));

    EXPECT_EQ(json.at("consumed").at("year").get<int>(), 2024);
    EXPECT_EQ(json.at("consumed").at("month").get<int>(), 3);
}

TEST(MediaEntrySerializerTest, MissingIdIsGenerated)
{
    auto json = MediaEntrySerializer::toJson(test::makeMovie("Heat", 2024, 3));
    json.erase("id");

    const auto first = MediaEntrySerializer::fromJson(json);
    const auto second = MediaEntrySerializer::fromJson(json);

    EXPECT_FALSE(first.id.empty());
    EXPECT_NE(first.id, second.id);
}

TEST(MediaEntrySerializerTest, UnknownCategoryIsRejected)
{
    auto json = MediaEntrySerializer::toJson(test::makeMovie("Heat", 2024, 3));
    json["category"] = "podcast";

    EXPECT_THROW(std::ignore = MediaEntrySerializer::fromJson(json), std::invalid_argument);
}

TEST(MediaEntrySerializerTest, InvalidConsumedMonthIsRejected)
{
    auto json = MediaEntrySerializer::toJson(test::makeMovie("Heat", 2024, 3));
    json["consumed"]["month"] = 13;

    EXPECT_THROW(std::ignore = MediaEntrySerializer::fromJson(json), std::invalid_argument);
}


TEST(MediaEntrySerializerTest, RatingAndNoteRoundTrip)
{
    auto entry = test::makeMovie("Heat", 2024, 3, "a");
    entry.rating = 8;
    entry.note = "Rewatch\nwith friends";

    const auto restored = MediaEntrySerializer::fromJson(MediaEntrySerializer::toJson(entry));

    EXPECT_EQ(restored.rating, 8);
    EXPECT_EQ(restored.note, "Rewatch\nwith friends");
}

TEST(MediaEntrySerializerTest, EntryWithoutRatingAndNoteWritesNeitherField)
{
    const auto json = MediaEntrySerializer::toJson(test::makeMovie("Heat", 2024, 3, "a"));

    EXPECT_FALSE(json.contains("rating"));
    EXPECT_FALSE(json.contains("note"));
}

TEST(MediaEntrySerializerTest, EntryOfAnOlderFileHasNoRatingAndNote)
{
    auto json = MediaEntrySerializer::toJson(test::makeMovie("Heat", 2024, 3, "a"));
    json["rating"] = nullptr;
    json["note"] = nullptr;

    const auto restored = MediaEntrySerializer::fromJson(json);

    EXPECT_FALSE(restored.rating.has_value());
    EXPECT_TRUE(restored.note.empty());
}

TEST(MediaEntrySerializerTest, RatingOutsideTheScaleMakesTheEntryUnreadable)
{
    auto json = MediaEntrySerializer::toJson(test::makeMovie("Heat", 2024, 3, "a"));

    json["rating"] = 0;
    EXPECT_ANY_THROW(std::ignore = MediaEntrySerializer::fromJson(json));

    json["rating"] = 11;
    EXPECT_ANY_THROW(std::ignore = MediaEntrySerializer::fromJson(json));
}
