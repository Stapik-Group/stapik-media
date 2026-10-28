#include "infrastructure/storage/MediaRepository.hpp"

#include "support/EntryFactory.hpp"
#include "support/FakeClock.hpp"
#include "support/TemporaryDirectory.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace
{
    namespace fs = std::filesystem;
    using namespace std::chrono_literals;

    MediaDocument documentWithMovie(const std::string& title, const std::string& id, const int year = 2026)
    {
        MediaDocument document;
        document.entries = { test::makeMovie(title, year, 3, id) };
        return document;
    }

    std::string readText(const fs::path& path)
    {
        std::ifstream stream(path);
        return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
    }

    void writeText(const fs::path& path, const std::string& text)
    {
        std::ofstream stream(path);
        stream << text;
    }

    class MediaRepositoryTest : public testing::Test
    {
    protected:
        stapik::test::TemporaryDirectory m_directory{ "stapik-media-repository" };
        stapik::test::FakeClock m_clock{ std::chrono::system_clock::now() };
    };
}

TEST_F(MediaRepositoryTest, LoadingWithoutAFileGivesAnEmptyDocument)
{
    MediaRepository repository(m_directory.path(), m_clock);

    const auto document = repository.load();

    EXPECT_TRUE(document.entries.empty());
}

TEST_F(MediaRepositoryTest, SavedDocumentIsLoadedBack)
{
    MediaRepository repository(m_directory.path(), m_clock);

    ASSERT_TRUE(repository.save(documentWithMovie("Heat", "movie-1")));

    const auto loaded = repository.load();
    ASSERT_EQ(loaded.entries.size(), 1U);
    EXPECT_EQ(loaded.entries[0].title, "Heat");
    EXPECT_EQ(repository.problem(), StorageProblem::None);
}

TEST_F(MediaRepositoryTest, ArchivesAreKeptInOneFilePerYear)
{
    MediaRepository repository(m_directory.path(), m_clock);

    ASSERT_TRUE(repository.saveArchive(2023, documentWithMovie("Heat", "movie-1", 2023)));
    ASSERT_TRUE(repository.saveArchive(2024, documentWithMovie("Alien", "movie-2", 2024)));

    EXPECT_TRUE(fs::exists(m_directory.path() / "media-2023.json"));
    EXPECT_TRUE(fs::exists(m_directory.path() / "media-2024.json"));

    const auto archives = repository.loadArchives();
    ASSERT_EQ(archives.size(), 2U);
    EXPECT_EQ(archives.at(2023).entries.at(0).title, "Heat");
    EXPECT_EQ(archives.at(2024).entries.at(0).title, "Alien");
}

TEST_F(MediaRepositoryTest, FilesThatAreNotArchivesAreIgnored)
{
    MediaRepository repository(m_directory.path(), m_clock);
    ASSERT_TRUE(repository.save(documentWithMovie("Heat", "movie-1")));
    writeText(m_directory.path() / "media-abcd.json", "{}");
    writeText(m_directory.path() / "media-2023.json.bak", "{}");

    EXPECT_TRUE(repository.loadArchives().empty());
}

TEST_F(MediaRepositoryTest, BackupIsNotRotatedAgainWithinTheInterval)
{
    MediaRepository repository(m_directory.path(), m_clock);

    ASSERT_TRUE(repository.save(documentWithMovie("First", "movie-1")));
    ASSERT_TRUE(repository.save(documentWithMovie("Second", "movie-2")));
    ASSERT_EQ(repository.listBackups().size(), 1U);

    m_clock.advance(1min);
    ASSERT_TRUE(repository.save(documentWithMovie("Third", "movie-3")));
    m_clock.advance(1min);
    ASSERT_TRUE(repository.save(documentWithMovie("Fourth", "movie-4")));

    EXPECT_EQ(repository.listBackups().size(), 1U);
    EXPECT_EQ(repository.load().entries.at(0).title, "Fourth");
}

TEST_F(MediaRepositoryTest, BackupIsRotatedOnceTheIntervalHasPassed)
{
    MediaRepository repository(m_directory.path(), m_clock);

    ASSERT_TRUE(repository.save(documentWithMovie("First", "movie-1")));
    ASSERT_TRUE(repository.save(documentWithMovie("Second", "movie-2")));
    ASSERT_EQ(repository.listBackups().size(), 1U);

    m_clock.advance(6min);
    ASSERT_TRUE(repository.save(documentWithMovie("Third", "movie-3")));

    EXPECT_EQ(repository.listBackups().size(), 2U);
}

TEST_F(MediaRepositoryTest, BackupHoldsTheDocumentThatWasReplaced)
{
    MediaRepository repository(m_directory.path(), m_clock);
    ASSERT_TRUE(repository.save(documentWithMovie("First", "movie-1")));
    ASSERT_TRUE(repository.save(documentWithMovie("Second", "movie-2")));

    const auto backups = repository.listBackups();
    ASSERT_EQ(backups.size(), 1U);

    const auto restored = repository.loadBackup(backups.front());
    ASSERT_EQ(restored.status, stapik::document::LoadStatus::Loaded);
    EXPECT_EQ(restored.document->entries.at(0).title, "First");
}

TEST_F(MediaRepositoryTest, FileFromANewerVersionIsKeptAndNotOverwritten)
{
    const auto path = m_directory.path() / "media.json";
    const std::string original = R"({"schemaVersion": 99, "document": {"entries": []}})";
    writeText(path, original);
    MediaRepository repository(m_directory.path(), m_clock);

    const auto loaded = repository.load();

    EXPECT_TRUE(loaded.entries.empty());
    EXPECT_EQ(repository.problem(), StorageProblem::NewerVersion);
    EXPECT_FALSE(repository.isWritable());
    EXPECT_FALSE(repository.save(documentWithMovie("Heat", "movie-1")));
    EXPECT_FALSE(repository.saveArchive(2023, documentWithMovie("Heat", "movie-1", 2023)));
    EXPECT_EQ(readText(path), original);
}

TEST_F(MediaRepositoryTest, FailedSaveReportsWhyAndWhere)
{
    const auto blocker = m_directory.path() / "blocker";
    writeText(blocker, "a file where a directory is needed");
    MediaRepository repository(blocker, m_clock);

    const auto saved = repository.save(documentWithMovie("Heat", "movie-1"));

    ASSERT_FALSE(saved);
    EXPECT_FALSE(saved.error().message().empty());
}
