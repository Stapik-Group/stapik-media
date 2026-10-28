#include "core/model/MediaModel.hpp"

#include "infrastructure/storage/MediaRepository.hpp"

#include "support/EntryFactory.hpp"
#include "support/FakeClock.hpp"
#include "support/FakeCloudStorage.hpp"
#include "support/ForwardingCloudStorage.hpp"
#include "support/MainLoopPump.hpp"
#include "support/TemporaryDirectory.hpp"

#include "stapik/domain/YearMonth.hpp"
#include "stapik/sync/CloudSessionHooks.hpp"
#include "stapik/sync/PartitionKey.hpp"
#include "stapik/sync/SyncEnvelope.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <optional>
#include <utility>

namespace
{
    using namespace std::chrono_literals;
    using stapik::test::pumpUntil;

    std::chrono::system_clock::time_point at(const int seconds)
    {
        using namespace std::chrono;
        return sys_days{ year{ 2026 } / January / 1 } + hours{ 12 } + std::chrono::seconds{ seconds };
    }

    CloudDocument cloudDocumentOf(const MediaDocument& document, const int serverSeconds)
    {
        return CloudDocument{
            .content = stapik::sync::SyncEnvelope{ .lastUpdate = document.lastUpdate(), .payload = document.toJson() }.toJson(),
            .updatedAt = at(serverSeconds)
        };
    }

    CloudStorageConfig validConfig()
    {
        return CloudStorageConfig{ .apiUrl = "https://example.com", .apiKey = "key" };
    }

    class MediaModelCloudTest : public testing::Test
    {
    protected:
        MediaModelCloudTest() :
            m_repository(m_directory.path(), m_clock),
            m_year(stapik::domain::YearMonth::current(m_clock).year())
        {
            m_fake.nextServerTime = at(1000);
            m_fake.serverTimeStep = 1s;
        }

        void SetUp() override
        {
            stapik::sync::CloudSessionHooks hooks;
            hooks.loadConfig = [] { return std::optional<CloudStorageConfig>{}; };
            hooks.saveConfig = [](const CloudStorageConfig&) { return true; };
            hooks.createStorage = [this](const CloudStorageConfig&) { return std::make_unique<stapik::test::ForwardingCloudStorage>(m_fake); };

            m_model = std::make_unique<MediaModel>(m_repository.load(), m_repository, std::move(hooks), stapik::sync::AsyncSyncOptions{ 20ms, 20ms, 80ms }, m_clock);
            m_model->signalAvailableYearsChanged().connect([this] { m_yearsKnown = true; });
        }

        void TearDown() override
        {
            m_model.reset();
        }

        stapik::test::TemporaryDirectory m_directory{ "stapik-media-cloud" };
        stapik::test::FakeClock m_clock{ std::chrono::system_clock::now() };
        stapik::test::FakeCloudStorage m_fake;
        MediaRepository m_repository;
        int m_year;
        bool m_yearsKnown = false;
        std::unique_ptr<MediaModel> m_model;
    };
}

TEST_F(MediaModelCloudTest, ConnectingUploadsTheMainDocument)
{
    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year, 3, "a")));

    ASSERT_TRUE(m_model->connectCloud(validConfig()));
    ASSERT_TRUE(pumpUntil([this] { return m_fake.saveCalls >= 1 && !m_model->isSyncing(); }));

    EXPECT_TRUE(m_model->isCloudConnected());
    ASSERT_TRUE(m_fake.stored.has_value());
    EXPECT_EQ(m_fake.stored->content.at("payload").at("entries").size(), 1U);
}

TEST_F(MediaModelCloudTest, IncompleteConfigurationIsRejected)
{
    EXPECT_FALSE(m_model->connectCloud(CloudStorageConfig{ .apiUrl = "https://example.com", .apiKey = "" }));
    EXPECT_FALSE(m_model->isCloudConnected());
}

TEST_F(MediaModelCloudTest, ChangeToAnArchivedYearIsSentToThatPartition)
{
    const auto pastYear = m_year - 2;
    ASSERT_TRUE(m_model->connectCloud(validConfig()));
    ASSERT_TRUE(pumpUntil([this] { return m_yearsKnown; }));

    ASSERT_TRUE(m_model->addEntry(test::makeMovie("Heat", pastYear, 3, "a")));

    ASSERT_TRUE(pumpUntil([this] { return m_fake.partitionSaveCalls >= 1; }));
    EXPECT_TRUE(m_fake.partitions.contains(stapik::sync::partitionKeyForYear(pastYear)));
}

TEST_F(MediaModelCloudTest, EntryOfAYearKeptOnlyInTheCloudIsRefusedUntilThatYearIsLoaded)
{
    const auto pastYear = m_year - 2;
    MediaDocument remote;
    remote.entries = { test::makeMovie("Heat", pastYear, 3, "remote") };
    remote.updatedAt = at(500);
    m_fake.partitions[stapik::sync::partitionKeyForYear(pastYear)] = cloudDocumentOf(remote, 600);

    ASSERT_TRUE(m_model->connectCloud(validConfig()));
    ASSERT_TRUE(pumpUntil([this] { return m_yearsKnown; }));

    EXPECT_FALSE(m_model->addEntry(test::makeMovie("Alien", pastYear, 5, "local")));
    EXPECT_TRUE(m_model->availableYears().contains(static_cast<unsigned>(pastYear)));

    m_model->requestYear(pastYear);
    ASSERT_TRUE(pumpUntil([this] { return m_model->entries().size() == 1; }));

    EXPECT_TRUE(m_model->addEntry(test::makeMovie("Alien", pastYear, 5, "local")));
    EXPECT_EQ(m_model->entries().size(), 2U);
}

TEST_F(MediaModelCloudTest, EntryOfAYearUnknownToTheCloudIsAccepted)
{
    ASSERT_TRUE(m_model->connectCloud(validConfig()));
    ASSERT_TRUE(pumpUntil([this] { return m_yearsKnown; }));

    EXPECT_TRUE(m_model->addEntry(test::makeMovie("Heat", m_year - 4, 3, "a")));
}
