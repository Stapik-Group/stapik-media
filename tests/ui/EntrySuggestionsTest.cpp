#include "ui/util/EntrySuggestions.hpp"

#include "support/EntryFactory.hpp"

#include <gtest/gtest.h>

namespace
{
    using test::makeEntry;

    MediaEntry movie(const std::string& director, const std::string& genre, const MediaCategory category = MediaCategory::Movie)
    {
        return makeEntry(category, "Title", 2024, 3, ScreenDetails{ director, genre });
    }

    MediaEntry book(const std::string& author, const std::string& genre)
    {
        return makeEntry(MediaCategory::Book, "Title", 2024, 3, BookDetails{ author, genre, false });
    }

    using Values = std::vector<std::string>;
}

TEST(EntrySuggestionsTest, NoEntriesGiveNoSuggestions)
{
    const auto lists = EntrySuggestions::collect({}, MediaCategory::Movie);

    EXPECT_TRUE(lists.creators.empty());
    EXPECT_TRUE(lists.publishers.empty());
    EXPECT_TRUE(lists.genres.empty());
    EXPECT_TRUE(lists.platforms.empty());
}

TEST(EntrySuggestionsTest, ValuesAreRankedByHowOftenTheyAreUsed)
{
    const std::vector entries{ movie("Mann", "Crime"), movie("Nolan", "Drama"), movie("Nolan", "Crime"), movie("Nolan", "Crime") };

    const auto lists = EntrySuggestions::collect(entries, MediaCategory::Movie);

    EXPECT_EQ(lists.creators, (Values{ "Nolan", "Mann" }));
    EXPECT_EQ(lists.genres, (Values{ "Crime", "Drama" }));
}

TEST(EntrySuggestionsTest, EqualCountsAreOrderedAlphabetically)
{
    const std::vector entries{ movie("Zemeckis", ""), movie("allen", ""), movie("Mann", "") };

    EXPECT_EQ(EntrySuggestions::collect(entries, MediaCategory::Movie).creators, (Values{ "allen", "Mann", "Zemeckis" }));
}

TEST(EntrySuggestionsTest, SpellingsThatDifferInCaseAreMergedUnderTheMostCommonOne)
{
    const std::vector entries{ movie("", "sci-fi"), movie("", "Sci-Fi"), movie("", "Sci-Fi"), movie("", " SCI-FI ") };

    EXPECT_EQ(EntrySuggestions::collect(entries, MediaCategory::Movie).genres, (Values{ "Sci-Fi" }));
}

TEST(EntrySuggestionsTest, EmptyValuesAreSkipped)
{
    const std::vector entries{ movie("", "  "), movie("Mann", "") };

    const auto lists = EntrySuggestions::collect(entries, MediaCategory::Movie);

    EXPECT_EQ(lists.creators, (Values{ "Mann" }));
    EXPECT_TRUE(lists.genres.empty());
}

TEST(EntrySuggestionsTest, MoviesSeriesAndCartoonsShareTheirSuggestions)
{
    const std::vector entries{ movie("Mann", "Crime", MediaCategory::Movie), movie("Odar", "Mystery", MediaCategory::Series), movie("Docter", "Family", MediaCategory::Cartoon) };

    EXPECT_EQ(EntrySuggestions::collect(entries, MediaCategory::Cartoon).creators.size(), 3U);
}

TEST(EntrySuggestionsTest, OtherKindsOfEntriesAreNotSuggested)
{
    const std::vector entries{ movie("Mann", "Crime"), book("Herbert", "Sci-Fi") };

    EXPECT_EQ(EntrySuggestions::collect(entries, MediaCategory::Movie).creators, (Values{ "Mann" }));
    EXPECT_EQ(EntrySuggestions::collect(entries, MediaCategory::Book).creators, (Values{ "Herbert" }));
}

TEST(EntrySuggestionsTest, AlbumsAndGamesFillTheirOwnFields)
{
    const std::vector entries{
        makeEntry(MediaCategory::Album, "Thriller", 2024, 3, AlbumDetails{ "Michael Jackson", "Epic", "Pop" }),
        makeEntry(MediaCategory::Game, "Doom", 2024, 3, GameDetails{ "id Software", "Bethesda", "PC" })
    };

    const auto albums = EntrySuggestions::collect(entries, MediaCategory::Album);
    const auto games = EntrySuggestions::collect(entries, MediaCategory::Game);

    EXPECT_EQ(albums.creators, (Values{ "Michael Jackson" }));
    EXPECT_EQ(albums.publishers, (Values{ "Epic" }));
    EXPECT_EQ(albums.genres, (Values{ "Pop" }));
    EXPECT_TRUE(albums.platforms.empty());
    EXPECT_EQ(games.creators, (Values{ "id Software" }));
    EXPECT_EQ(games.publishers, (Values{ "Bethesda" }));
    EXPECT_EQ(games.platforms, (Values{ "PC" }));
    EXPECT_TRUE(games.genres.empty());
}
