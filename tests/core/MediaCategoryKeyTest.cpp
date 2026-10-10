#include "core/model/MediaCategoryKey.hpp"

#include <gtest/gtest.h>

#include <array>

namespace
{
    constexpr std::array ALL_CATEGORIES = {
        MediaCategory::Movie, MediaCategory::Series, MediaCategory::Cartoon,
        MediaCategory::Book, MediaCategory::Album, MediaCategory::Game
    };
}

TEST(MediaCategoryKeyTest, EveryCategoryRoundTrips)
{
    for (const auto category : ALL_CATEGORIES)
    {
        const auto parsed = categoryFromKey(categoryToKey(category));

        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(*parsed, category);
    }
}

TEST(MediaCategoryKeyTest, KeysAreStable)
{
    EXPECT_EQ(categoryToKey(MediaCategory::Movie), "movie");
    EXPECT_EQ(categoryToKey(MediaCategory::Album), "album");
    EXPECT_EQ(categoryToKey(MediaCategory::Game), "game");
}

TEST(MediaCategoryKeyTest, UnknownKeyIsRejected)
{
    EXPECT_FALSE(categoryFromKey("podcast").has_value());
    EXPECT_FALSE(categoryFromKey("").has_value());
}
