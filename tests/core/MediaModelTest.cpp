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
#include <iterator>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace
{
    namespace fs = std::filesystem;

    MediaEntry releasedIn(MediaEntry entry, const int year)
    {
        entry.releaseDate = ReleaseDate{ std::chrono::year{ year } / std::chrono::month{ 1 } / std::chrono::day{ 1 }, DatePrecision::Day };
        return entry;
    }

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

TEST_F(MediaModelTest, TitleAlreadyInTheCategoryIsRefused)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    const auto second = m_model->addEntry(test::makeMovie("Heat", m_year, 5, "b"));

    ASSERT_FALSE(second);
    EXPECT_EQ(second.error(), MediaModel::EntryError::Duplicate);
    EXPECT_EQ(idsOf(m_model->entries()), std::vector<std::string>{ "a" });
    EXPECT_EQ(m_repository.load().entries.size(), 1U);
}

TEST_F(MediaModelTest, TitleComparisonIgnoresCaseAndSurroundingSpaces)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    const auto second = m_model->addEntry(test::makeMovie("  hEAT ", m_year, 5, "b"));

    ASSERT_FALSE(second);
    EXPECT_EQ(second.error(), MediaModel::EntryError::Duplicate);
}

TEST_F(MediaModelTest, SameTitleInAnotherCategoryIsAccepted)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Dune", m_year, 3, "a")));

    EXPECT_TRUE(m_model->addEntry(test::makeBook("Dune", m_year, 3, "b")));
}

TEST_F(MediaModelTest, DuplicateIsFoundAmongArchivedYears)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year - 2, 3, "a")));

    const auto second = m_model->addEntry(test::makeMovie("Heat", m_year, 3, "b"));

    ASSERT_FALSE(second);
    EXPECT_EQ(second.error(), MediaModel::EntryError::Duplicate);
}

TEST_F(MediaModelTest, SameTitleReleasedInAnotherYearIsAccepted)
{
    ASSERT_TRUE(m_model->addEntry(releasedIn(test::makeMovie("Superman", m_year, 3, "a"), 1978)));

    EXPECT_TRUE(m_model->addEntry(releasedIn(test::makeMovie("Superman", m_year, 3, "b"), 2025)));
    EXPECT_EQ(m_model->entries().size(), 2U);
}

TEST_F(MediaModelTest, SameTitleReleasedInTheSameYearIsRefusedEvenWithADifferentDay)
{
    ASSERT_TRUE(m_model->addEntry(releasedIn(test::makeMovie("Superman", m_year, 3, "a"), 1978)));

    const auto second = m_model->addEntry(releasedIn(test::makeMovie("Superman", m_year, 4, "b"), 1978));

    ASSERT_FALSE(second);
    EXPECT_EQ(second.error(), MediaModel::EntryError::Duplicate);
}

TEST_F(MediaModelTest, ChangingTheReleaseYearToTheOneOfAnotherEntryIsRefused)
{
    ASSERT_TRUE(m_model->addEntry(releasedIn(test::makeMovie("Superman", m_year, 3, "a"), 1978)));
    ASSERT_TRUE(m_model->addEntry(releasedIn(test::makeMovie("Superman", m_year, 4, "b"), 2025)));

    const auto changed = m_model->updateEntry("b", releasedIn(test::makeMovie("Superman", m_year, 4, "b"), 1978));

    ASSERT_FALSE(changed);
    EXPECT_EQ(changed.error(), MediaModel::EntryError::Duplicate);
}

TEST_F(MediaModelTest, RenamingAnEntryToAnExistingTitleIsRefused)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year, 4, "b")));

    const auto renamed = m_model->updateEntry("b", test::makeMovie("Heat", m_year, 4, "b"));

    ASSERT_FALSE(renamed);
    EXPECT_EQ(renamed.error(), MediaModel::EntryError::Duplicate);
    EXPECT_EQ(m_model->entryById("b")->title, "Alien");
}

TEST_F(MediaModelTest, EditingAnEntryWithoutChangingItsTitleIsAccepted)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    EXPECT_TRUE(m_model->updateEntry("a", test::makeMovie("Heat", m_year, 9, "a")));
    EXPECT_EQ(m_model->entryById("a")->consumed.month(), 9);
}

TEST_F(MediaModelTest, EntriesThatAlreadyShareATitleCanStillBeEdited)
{
    MediaDocument legacy;
    legacy.entries = { test::makeMovie("Heat", m_year, 1, "a"), test::makeMovie("Heat", m_year, 2, "b") };
    m_model = std::make_unique<MediaModel>(std::move(legacy), m_repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, m_clock);

    EXPECT_TRUE(m_model->updateEntry("a", test::makeMovie("Heat", m_year, 6, "a")));
    EXPECT_EQ(m_model->entries().size(), 2U);
}

TEST_F(MediaModelTest, UpdatingAnUnknownEntryIsReported)
{
    const auto updated = m_model->updateEntry("missing", test::makeMovie("Heat", m_year, 3, "missing"));

    ASSERT_FALSE(updated);
    EXPECT_EQ(updated.error(), MediaModel::EntryError::UnknownEntry);
}

TEST_F(MediaModelTest, IdenticalEntriesOfAnEarlierYearAreArchivedOnce)
{
    MediaDocument legacy;
    legacy.entries = { test::makeMovie("Heat", m_year - 3, 5, "a"), test::makeMovie("Heat", m_year - 3, 5, "b") };

    m_model = std::make_unique<MediaModel>(std::move(legacy), m_repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, m_clock);

    EXPECT_EQ(m_model->entries().size(), 1U);
    EXPECT_EQ(m_repository.loadArchives().at(m_year - 3).entries.size(), 1U);
}

TEST_F(MediaModelTest, EntriesWithTheSameTitleButAnotherMonthAreBothArchived)
{
    MediaDocument legacy;
    legacy.entries = { test::makeMovie("Heat", m_year - 3, 3, "a"), test::makeMovie("Heat", m_year - 3, 5, "b") };

    m_model = std::make_unique<MediaModel>(std::move(legacy), m_repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, m_clock);

    EXPECT_EQ(m_model->entries().size(), 2U);
    EXPECT_EQ(m_repository.loadArchives().at(m_year - 3).entries.size(), 2U);
}

TEST_F(MediaModelTest, UnreadableEntriesAreCountedAndKeptWhenTheListIsSaved)
{
    MediaDocument stored;
    stored.entries = { test::makeMovie("Heat", m_year, 3, "a") };
    stored.unreadableEntries = { nlohmann::json{ { "category", "podcast" } } };
    m_model = std::make_unique<MediaModel>(std::move(stored), m_repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, m_clock);
    EXPECT_EQ(m_model->unreadableEntryCount(), 1U);

    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year, 4, "b")));

    EXPECT_EQ(m_model->entries().size(), 2U);
    EXPECT_EQ(m_repository.load().unreadableEntries.size(), 1U);
}

TEST_F(MediaModelTest, YearThatEndedIsArchivedWithoutRestartingTheProgram)
{
    using namespace std::chrono;
    m_clock.set(sys_days{ year{ m_year } / December / 31 } + hours{ 12 });
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 12, "a")));
    ASSERT_TRUE(m_repository.loadArchives().empty());

    m_clock.set(sys_days{ year{ m_year + 1 } / January / 2 } + hours{ 12 });
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year, 12, "b")));

    EXPECT_TRUE(m_repository.load().entries.empty());
    ASSERT_TRUE(m_repository.loadArchives().contains(m_year));
    EXPECT_EQ(m_repository.loadArchives().at(m_year).entries.size(), 2U);
    EXPECT_EQ(m_model->entries().size(), 2U);
}

TEST_F(MediaModelTest, ArchivingTheYearThatEndedIsReportedToTheList)
{
    using namespace std::chrono;
    m_clock.set(sys_days{ year{ m_year } / December / 31 } + hours{ 12 });
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 12, "a")));
    int changes = 0;
    m_model->signalEntriesChanged().connect([&changes] { ++changes; });

    m_clock.set(sys_days{ year{ m_year + 1 } / January / 2 } + hours{ 12 });
    m_model->removeEntry("a");

    EXPECT_EQ(changes, 2);
    EXPECT_TRUE(m_model->entries().empty());
}

TEST_F(MediaModelTest, NoExtraChangeIsReportedWhileTheYearStaysTheSame)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));
    int changes = 0;
    m_model->signalEntriesChanged().connect([&changes] { ++changes; });

    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year, 4, "b")));

    EXPECT_EQ(changes, 1);
}

TEST_F(MediaModelTest, TurnOfTheYearWithoutEntriesToMoveChangesNothing)
{
    using namespace std::chrono;
    m_clock.set(sys_days{ year{ m_year } / December / 31 } + hours{ 12 });
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year + 1, 1, "a")));
    int changes = 0;
    m_model->signalEntriesChanged().connect([&changes] { ++changes; });

    m_clock.set(sys_days{ year{ m_year + 1 } / January / 2 } + hours{ 12 });
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year + 1, 2, "b")));

    EXPECT_EQ(changes, 1);
    EXPECT_TRUE(m_repository.loadArchives().empty());
}

TEST_F(MediaModelTest, EntriesOfTheCurrentYearStayIntactWhenNothingIsArchived)
{
    MediaDocument stored;
    stored.entries = { test::makeMovie("Heat", m_year, 3, "a"), test::makeBook("Dune", m_year, 4, "b") };

    m_model = std::make_unique<MediaModel>(std::move(stored), m_repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, m_clock);

    ASSERT_EQ(m_model->entries().size(), 2U);
    EXPECT_EQ(m_model->entries()[0].title, "Heat");
    EXPECT_EQ(m_model->entries()[0].id, "a");
    EXPECT_EQ(std::get<ScreenDetails>(m_model->entries()[0].details).director, "Director");
    EXPECT_EQ(m_model->entries()[1].title, "Dune");
    EXPECT_EQ(m_model->entries()[1].id, "b");
    EXPECT_EQ(std::get<BookDetails>(m_model->entries()[1].details).author, "Author");
}

TEST_F(MediaModelTest, EntriesOfTheCurrentYearAreWrittenWithTheirContentAfterTheNextChange)
{
    MediaDocument stored;
    stored.entries = { test::makeMovie("Heat", m_year, 3, "a") };
    m_model = std::make_unique<MediaModel>(std::move(stored), m_repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, m_clock);

    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year, 4, "b")));

    const auto saved = m_repository.load();
    ASSERT_EQ(saved.entries.size(), 2U);
    EXPECT_EQ(saved.entries[0].title, "Heat");
    EXPECT_EQ(saved.entries[0].id, "a");
    EXPECT_EQ(saved.entries[1].title, "Alien");
}

TEST_F(MediaModelTest, EntriesOfTheNewCurrentYearStayIntactWhenTheYearTurns)
{
    using namespace std::chrono;
    m_clock.set(sys_days{ year{ m_year } / December / 31 } + hours{ 12 });
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year + 1, 1, "a")));

    m_clock.set(sys_days{ year{ m_year + 1 } / January / 2 } + hours{ 12 });
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year + 1, 2, "b")));

    ASSERT_EQ(m_model->entries().size(), 2U);
    EXPECT_EQ(m_model->entries()[0].title, "Heat");
    EXPECT_EQ(m_model->entries()[0].id, "a");
    EXPECT_EQ(m_repository.load().entries.at(0).title, "Heat");
}

TEST_F(MediaModelTest, ImportAddsNewEntriesWithGeneratedIds)
{
    const auto added = m_model->importEntries({ test::makeMovie("Heat", m_year, 3, ""), test::makeBook("Dune", m_year, 4, "") });

    EXPECT_EQ(added, 2U);
    ASSERT_EQ(m_model->entries().size(), 2U);
    EXPECT_FALSE(m_model->entries()[0].id.empty());
    EXPECT_NE(m_model->entries()[0].id, m_model->entries()[1].id);
    EXPECT_EQ(m_repository.load().entries.size(), 2U);
}

TEST_F(MediaModelTest, ImportSkipsWorksThatAreAlreadyInTheList)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    const auto added = m_model->importEntries({ test::makeMovie("  HEAT ", m_year, 5, ""), test::makeMovie("Alien", m_year, 5, "") });

    EXPECT_EQ(added, 1U);
    EXPECT_EQ(m_model->entries().size(), 2U);
}

TEST_F(MediaModelTest, ImportSkipsWorksRepeatedInsideTheImportedFile)
{
    const auto candidates = std::vector<MediaEntry>{ test::makeMovie("Heat", m_year, 3, ""), test::makeMovie("heat", m_year, 4, ""), test::makeBook("Heat", m_year, 4, "") };

    const auto verdicts = m_model->previewImport(candidates);
    const auto added = m_model->importEntries(candidates);

    EXPECT_EQ(verdicts, (std::vector<MediaModel::ImportVerdict>{ MediaModel::ImportVerdict::Add, MediaModel::ImportVerdict::Duplicate, MediaModel::ImportVerdict::Add }));
    EXPECT_EQ(added, 2U);
}

TEST_F(MediaModelTest, ImportOfNothingNewChangesNothing)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));
    m_model->undoStack().clear();

    EXPECT_EQ(m_model->importEntries({ test::makeMovie("Heat", m_year, 3, "") }), 0U);
    EXPECT_FALSE(m_model->undoStack().canUndo());
}

TEST_F(MediaModelTest, ImportPutsEntriesOfEarlierYearsInTheirArchives)
{
    EXPECT_EQ(m_model->importEntries({ test::makeMovie("Heat", m_year, 3, ""), test::makeMovie("Alien", m_year - 2, 5, "") }), 2U);

    EXPECT_EQ(m_repository.load().entries.size(), 1U);
    const auto archives = m_repository.loadArchives();
    ASSERT_TRUE(archives.contains(m_year - 2));
    EXPECT_EQ(archives.at(m_year - 2).entries.size(), 1U);
}

TEST_F(MediaModelTest, WholeImportIsUndoneAndRedoneInOneStep)
{
    ASSERT_EQ(m_model->importEntries({ test::makeMovie("Heat", m_year, 3, ""), test::makeMovie("Alien", m_year - 1, 5, ""), test::makeBook("Dune", m_year, 4, "") }), 3U);

    ASSERT_TRUE(m_model->undoStack().undo());
    EXPECT_TRUE(m_model->entries().empty());
    EXPECT_FALSE(m_model->undoStack().canUndo());

    ASSERT_TRUE(m_model->undoStack().redo());
    EXPECT_EQ(m_model->entries().size(), 3U);
}

TEST_F(MediaModelTest, SnapshotContainsTheEntriesOfAllLoadedYears)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year - 2, 5, "b")));

    const auto snapshot = m_model->snapshot();

    EXPECT_EQ(idsOf(snapshot.entries), (std::vector<std::string>{ "b", "a" }));
}

TEST_F(MediaModelTest, ReplaceAllReplacesEveryYearAndEmptiesTheYearsMissingFromTheNewData)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year - 2, 5, "b")));
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Ronin", m_year - 1, 7, "c")));

    MediaDocument imported;
    imported.entries = { test::makeBook("Dune", m_year, 1, "d"), test::makeMovie("Zodiac", m_year - 1, 2, "e") };

    ASSERT_TRUE(m_model->replaceAll(std::move(imported)));

    EXPECT_EQ(idsOf(m_model->entries()), (std::vector<std::string>{ "e", "d" }));
    EXPECT_EQ(idsOf(m_repository.load().entries), std::vector<std::string>{ "d" });
    const auto archives = m_repository.loadArchives();
    EXPECT_TRUE(archives.at(m_year - 2).entries.empty());
    EXPECT_EQ(idsOf(archives.at(m_year - 1).entries), std::vector<std::string>{ "e" });
}

TEST_F(MediaModelTest, ReplaceAllClearsTheUndoHistory)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    ASSERT_TRUE(m_model->replaceAll(MediaDocument{}));

    EXPECT_FALSE(m_model->undoStack().canUndo());
    EXPECT_TRUE(m_model->entries().empty());
}

TEST_F(MediaModelTest, ReplacedEntriesRoundTripThroughASnapshot)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Alien", m_year - 3, 5, "b")));
    auto snapshot = m_model->snapshot();

    ASSERT_TRUE(m_model->replaceAll(MediaDocument{}));
    ASSERT_TRUE(m_model->replaceAll(std::move(snapshot)));

    EXPECT_EQ(idsOf(m_model->entries()), (std::vector<std::string>{ "b", "a" }));
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

namespace
{
    constexpr auto NEWER_VERSION_FILE = R"({"schemaVersion": 99, "document": {"entries": []}})";
}

TEST(MediaModelReadOnlyTest, EntriesCannotBeChangedWhenTheDataFileIsFromANewerVersion)
{
    stapik::test::TemporaryDirectory directory{ "stapik-media-readonly" };
    stapik::test::FakeClock clock{ std::chrono::system_clock::now() };
    const auto path = directory.path() / "media.json";
    std::ofstream(path) << NEWER_VERSION_FILE;
    MediaRepository repository(directory.path(), clock);
    MediaModel model(repository.load(), repository, stapik::sync::CloudSessionHooks{}, stapik::sync::AsyncSyncOptions{}, clock);
    const auto year = stapik::domain::YearMonth::current(clock).year();

    const auto added = model.addEntry(test::makeMovie("Heat", year, 3, "a"));
    const auto updated = model.updateEntry("a", test::makeMovie("Heat", year, 4, "a"));
    model.removeEntry("a");

    EXPECT_FALSE(model.isWritable());
    ASSERT_FALSE(added);
    EXPECT_EQ(added.error(), MediaModel::EntryError::ReadOnly);
    ASSERT_FALSE(updated);
    EXPECT_EQ(updated.error(), MediaModel::EntryError::ReadOnly);
    EXPECT_TRUE(model.entries().empty());

    std::ifstream stream(path);
    const std::string content{ std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
    EXPECT_EQ(content, NEWER_VERSION_FILE);
}

TEST_F(MediaModelTest, ModelOfAWritableRepositoryIsWritable)
{
    EXPECT_TRUE(m_model->isWritable());
}
