#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace
{
    namespace fs = std::filesystem;

    const fs::path MEDIA_RESOURCES_DIR = fs::path(STAPIK_MEDIA_SOURCE_DIR) / "resources";
    const fs::path COMMON_RESOURCES_DIR = STAPIK_COMMON_RESOURCES_DIR;
    const std::vector<std::string> LANGUAGES = { "en", "pl", "de" };

    std::map<std::string, std::string> loadStrings(const fs::path& directory, const std::string& language)
    {
        const auto path = directory / "locales" / (language + ".json");
        std::ifstream file(path);
        EXPECT_TRUE(file.good()) << "missing " << path;
        return nlohmann::json::parse(file).get<std::map<std::string, std::string>>();
    }

    std::set<std::string> keysOf(const std::map<std::string, std::string>& strings)
    {
        std::set<std::string> keys;
        for (const auto& entry : strings)
            keys.insert(entry.first);

        return keys;
    }

    std::set<std::string> tutorialPagesOf(const std::string& language)
    {
        const auto root = MEDIA_RESOURCES_DIR / "tutorial" / language;
        std::set<std::string> pages;
        for (const auto& entry : fs::recursive_directory_iterator(root))
        {
            if (entry.path().extension() == ".md")
                pages.insert(fs::relative(entry.path(), root).generic_string());
        }

        return pages;
    }

    std::string firstLineOf(const fs::path& path)
    {
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line))
        {
            if (!line.empty())
                return line;
        }

        return {};
    }
}

TEST(MediaResourcesTest, EveryLanguageDefinesTheSameKeys)
{
    const auto expectedKeys = keysOf(loadStrings(MEDIA_RESOURCES_DIR, "en"));

    for (const auto& language : LANGUAGES)
        EXPECT_EQ(keysOf(loadStrings(MEDIA_RESOURCES_DIR, language)), expectedKeys) << "language " << language;
}

TEST(MediaResourcesTest, NoTranslationIsEmpty)
{
    for (const auto& language : LANGUAGES)
    {
        for (const auto& [key, value] : loadStrings(MEDIA_RESOURCES_DIR, language))
            EXPECT_FALSE(value.empty()) << key << " in " << language;
    }
}

TEST(MediaResourcesTest, MediaKeysDoNotShadowKeysOfStapikCommon)
{
    for (const auto& language : LANGUAGES)
    {
        const auto commonKeys = keysOf(loadStrings(COMMON_RESOURCES_DIR, language));

        for (const auto& [key, value] : loadStrings(MEDIA_RESOURCES_DIR, language))
            EXPECT_EQ(commonKeys.count(key), 0U) << key << " in " << language << " is already defined by stapik-common";
    }
}

TEST(MediaResourcesTest, EveryLanguageHasTheSameTutorialPages)
{
    const auto expectedPages = tutorialPagesOf("en");
    ASSERT_FALSE(expectedPages.empty());

    for (const auto& language : LANGUAGES)
        EXPECT_EQ(tutorialPagesOf(language), expectedPages) << "language " << language;
}

TEST(MediaResourcesTest, EveryTutorialPageStartsWithAHeading)
{
    for (const auto& language : LANGUAGES)
    {
        for (const auto& page : tutorialPagesOf(language))
            EXPECT_TRUE(firstLineOf(MEDIA_RESOURCES_DIR / "tutorial" / language / page).starts_with("# ")) << language << "/" << page;
    }
}
