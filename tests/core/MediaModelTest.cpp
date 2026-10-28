#include "core/model/MediaModel.hpp"

#include "infrastructure/storage/MediaRepository.hpp"

#include "support/EntryFactory.hpp"
#include "support/FakeClock.hpp"
#include "support/TemporaryDirectory.hpp"

#include "stapik/domain/YearMonth.hpp"
#include "stapik/sync/CloudSessionHooks.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace
{
    namespace fs = std::filesystem;

    std::vector<std::string> idsOf(const std::vector<MediaEntry>& entries)
    {
        std::vector<std::string> ids;
        for (const auto& entry : entries)
            ids.push_back(entry.id);

        return ids;
    }

    class MediaModelTest : public testing::Test
    {
    protected:
        MediaModelTest() :
            m_repository(m_directory.path(), m_clock),
            m_year(stapik::domain::YearMonth::current(m_clock).year())
        {
            m_model = std::make_unique<MediaModel>(m_repository.load(), m_repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, m_clock);
        }

        stapik::test::TemporaryDirectory m_directory{ "stapik-media-model" };
        stapik::test::FakeClock m_clock{ std::chrono::system_clock::now() };
        MediaRepository m_repository;
        int m_year;
        std::unique_ptr<MediaModel> m_model;
    };
}

TEST_F(MediaModelTest, EntryOfTheCurrentYearStaysInTheMainDocument)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    EXPECT_EQ(idsOf(m_model->entries()), std::vector<std::string>{ "a" });
    EXPECT_EQ(m_repository.load().entries.size(), 1U);
    EXPECT_TRUE(m_repository.loadArchives().empty());
}

TEST_F(MediaModelTest, EntryOfAnEarlierYearGoesToItsArchive)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year - 2, 3, "a")));

    EXPECT_TRUE(m_repository.load().entries.empty());
    const auto archives = m_repository.loadArchives();
    ASSERT_TRUE(archives.contains(m_year - 2));
    EXPECT_EQ(archives.at(m_year - 2).entries.size(), 1U);
    EXPECT_EQ(m_model->entries().size(), 1U);
}

TEST_F(MediaModelTest, UpdatingTheYearMovesTheEntryBetweenDocuments)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    ASSERT_TRUE(m_model->updateEntry("a", test::makeMovie("Heat", m_year - 1, 3, "a")));

    EXPECT_TRUE(m_repository.load().entries.empty());
    ASSERT_TRUE(m_repository.loadArchives().contains(m_year - 1));
    EXPECT_EQ(idsOf(m_model->entries()), std::vector<std::string>{ "a" });
}

TEST_F(MediaModelTest, RemovingAnEntryDeletesItFromDisk)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    m_model->removeEntry("a");

    EXPECT_TRUE(m_model->entries().empty());
    EXPECT_TRUE(m_repository.load().entries.empty());
}

TEST_F(MediaModelTest, UndoTakesBackAnAddedEntry)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    ASSERT_TRUE(m_model->undoStack().undo());

    EXPECT_TRUE(m_model->entries().empty());
    EXPECT_TRUE(m_repository.load().entries.empty());
}

TEST_F(MediaModelTest, EntriesOfEarlierYearsInTheMainDocumentAreMovedToArchivesOnStart)
{
    MediaDocument legacy;
    legacy.entries = { test::makeMovie("Old", m_year - 3, 5, "old"), test::makeMovie("New", m_year, 5, "new") };

    m_model = std::make_unique<MediaModel>(std::move(legacy), m_repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, m_clock);

    EXPECT_EQ(idsOf(m_repository.load().entries), std::vector<std::string>{ "new" });
    ASSERT_TRUE(m_repository.loadArchives().contains(m_year - 3));
    EXPECT_EQ(m_model->entries().size(), 2U);
}

TEST_F(MediaModelTest, RestoringABackupReplacesTheMainDocument)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("First", m_year, 1, "a")));
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Second", m_year, 2, "b")));
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Third", m_year, 3, "c")));

    const auto backups = m_model->listBackups();
    ASSERT_EQ(backups.size(), 1U);

    ASSERT_TRUE(m_model->restoreBackup(backups.front()));

    EXPECT_EQ(idsOf(m_model->entries()), std::vector<std::string>{ "a" });
    EXPECT_EQ(idsOf(m_repository.load().entries), std::vector<std::string>{ "a" });
    EXPECT_FALSE(m_model->undoStack().canUndo());
}

TEST_F(MediaModelTest, AnUnreadableBackupIsRefusedAndLeavesTheDocumentAlone)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("First", m_year, 1, "a")));
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Second", m_year, 2, "b")));

    const auto backups = m_model->listBackups();
    ASSERT_EQ(backups.size(), 1U);
    std::ofstream(backups.front().path) << "this is not json";

    EXPECT_FALSE(m_model->restoreBackup(backups.front()));
    EXPECT_EQ(m_model->entries().size(), 2U);
}

TEST(MediaModelSaveFailureTest, FailureIsReportedOnceUntilASaveSucceedsAgain)
{
    stapik::test::TemporaryDirectory directory{ "stapik-media-model-failure" };
    stapik::test::FakeClock clock{ std::chrono::system_clock::now() };
    const auto year = stapik::domain::YearMonth::current(clock).year();
    const auto blocker = directory.path() / "blocker";
    auto blockWithAFile = [&blocker]
    {
        std::error_code errorCode;
        fs::remove_all(blocker, errorCode);
        std::ofstream(blocker) << "a file where a directory is needed";
    };
    blockWithAFile();

    MediaRepository repository(blocker, clock);
    MediaModel model(repository.load(), repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, clock);
    std::vector<std::string> failures;
    model.signalSaveFailed().connect([&failures](const std::string& message) { failures.push_back(message); });

    ASSERT_TRUE(model.addEntry(test::makeMovie("First", year, 1, "a")));
    ASSERT_TRUE(model.addEntry(test::makeMovie("Second", year, 2, "b")));
    EXPECT_EQ(failures.size(), 1U);
    EXPECT_FALSE(failures.front().empty());

    std::error_code errorCode;
    fs::remove_all(blocker, errorCode);
    ASSERT_TRUE(model.addEntry(test::makeMovie("Third", year, 3, "c")));
    EXPECT_EQ(failures.size(), 1U);

    blockWithAFile();
    ASSERT_TRUE(model.addEntry(test::makeMovie("Fourth", year, 4, "d")));
    EXPECT_EQ(failures.size(), 2U);
}
