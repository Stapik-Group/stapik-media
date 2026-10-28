#include "MediaRepository.hpp"

#include "MediaDocumentSchema.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/PathText.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/sync/PartitionKey.hpp"

#include <format>
#include <system_error>
#include <utility>
#include <string_view>

namespace
{
    stapik::archive::AppArchiveContext archiveContext()
    {
        const auto& context = stapik::app::AppContext::instance();

        return {
            .appName = context.info().internalName,
            .appVersion = context.info().version,
            .settings = &context.settings(),
            .assets = nullptr
        };
    }
}

MediaRepository::MediaRepository() :
    MediaRepository(directoryPath())
{}

MediaRepository::MediaRepository(std::filesystem::path directory, const stapik::time::IClock& clock) :
    m_directory(std::move(directory)),
    m_file(m_directory / FILE_NAME, createMediaSchemaMigrator(), clock),
    m_clock(clock)
{
    m_file.setBackupCount(BACKUP_COUNT);
}

MediaDocument MediaRepository::load()
{
    auto result = m_file.load();

    switch (result.status)
    {
        using enum stapik::document::LoadStatus;
        case Loaded:
            if (result.migrated)
            {
                if (const auto saved = m_file.save(*result.document); !saved)
                    stapik::log::warning("Cannot save the migrated media document: {}", saved.error().message());
            }

            return std::move(*result.document);
        case Missing:
            return {};
        case Corrupted:
            raiseProblem(StorageProblem::Corrupted);
            return {};
        case NewerVersion:
            raiseProblem(StorageProblem::NewerVersion);
            return {};
        case MigrationFailed:
            raiseProblem(StorageProblem::MigrationFailed);
            return {};
    }

    return {};
}

std::map<int, MediaDocument> MediaRepository::loadArchives()
{
    std::map<int, MediaDocument> archives;

    std::error_code errorCode;
    for (const auto& item : std::filesystem::directory_iterator(m_directory, errorCode))
    {
        const auto year = archiveYearFrom(item.path().filename().string());
        if (!year.has_value())
            continue;

        const auto file = archiveFile(item.path());
        auto result = file.load();

        switch (result.status)
        {
            using enum stapik::document::LoadStatus;
            case Loaded:
                if (result.migrated)
                {
                    if (const auto saved = file.save(*result.document); !saved)
                        stapik::log::warning("Cannot save the migrated media archive {}: {}", *year, saved.error().message());
                }

                archives.emplace(*year, std::move(*result.document));
                break;
            case Missing:
                break;
            case Corrupted:
                raiseProblem(StorageProblem::Corrupted);
                break;
            case NewerVersion:
                raiseProblem(StorageProblem::NewerVersion);
                break;
            case MigrationFailed:
                raiseProblem(StorageProblem::MigrationFailed);
                break;
        }
    }

    return archives;
}

stapik::storage::FileResult MediaRepository::save(const MediaDocument& document) const
{
    if (!isWritable())
        return notWritable(m_file.filePath());

    return write(m_file, document);
}

stapik::storage::FileResult MediaRepository::saveArchive(const int year, const MediaDocument& document) const
{
    const auto path = archivePath(year);
    if (!isWritable())
        return notWritable(path);

    return write(archiveFile(path), document);
}

std::vector<stapik::document::BackupInfo> MediaRepository::listBackups() const
{
    return m_file.listBackups();
}

stapik::document::LoadResult<MediaDocument> MediaRepository::loadBackup(const stapik::document::BackupInfo& backup) const
{
    return m_file.loadBackup(backup);
}

MediaRepository::ArchiveInfoResult MediaRepository::exportArchive(const MediaDocument& document, const std::filesystem::path& target) const
{
    return stapik::archive::exportAppArchive(m_file, document, archiveContext(), target, m_clock);
}

MediaRepository::ArchiveInfoResult MediaRepository::importArchive(const std::filesystem::path& source, const std::function<bool(MediaDocument)>& apply) const
{
    const auto context = archiveContext();

    auto archive = stapik::archive::AppArchive::open(source, context.appName);
    if (!archive)
        return stapik::Failure(std::move(archive).error());

    auto loaded = m_file.deserialize(archive->documentText(), "the archive " + stapik::storage::pathText(source));
    switch (loaded.status)
    {
        using enum stapik::document::LoadStatus;
        case Loaded:
            break;
        case NewerVersion:
            return stapik::archive::archiveFailure(stapik::archive::ArchiveErrorKind::Unsupported,
                "The archive was made by a newer version of the application");
        case Missing:
        case Corrupted:
        case MigrationFailed:
        default:
            return stapik::archive::archiveFailure(stapik::archive::ArchiveErrorKind::Corrupted,
                "The document in the archive cannot be read");
    }

    if (!apply(std::move(*loaded.document)))
        return stapik::archive::archiveFailure(stapik::archive::ArchiveErrorKind::Io, "The entries from the archive could not be applied");

    if (context.settings != nullptr)
        static_cast<void>(archive->restoreSettings(*context.settings));

    return archive->info();
}

StorageProblem MediaRepository::problem() const
{
    return m_problem;
}

bool MediaRepository::isWritable() const
{
    return m_problem != StorageProblem::NewerVersion && m_problem != StorageProblem::MigrationFailed;
}

void MediaRepository::raiseProblem(const StorageProblem problem)
{
    if (m_problem == StorageProblem::None || (m_problem == StorageProblem::Corrupted && problem != StorageProblem::Corrupted))
        m_problem = problem;
}

stapik::document::DocumentFile<MediaDocument> MediaRepository::archiveFile(const std::filesystem::path& path) const
{
    stapik::document::DocumentFile<MediaDocument> file(path, createMediaSchemaMigrator(), m_clock);
    file.setBackupCount(BACKUP_COUNT);
    return file;
}

bool MediaRepository::isBackupDue(const stapik::document::DocumentFile<MediaDocument>& file) const
{
    const auto backups = file.listBackups();
    return backups.empty() || m_clock.now() - backups.front().savedAt >= BACKUP_INTERVAL;
}

stapik::storage::FileResult MediaRepository::write(const stapik::document::DocumentFile<MediaDocument>& file, const MediaDocument& document) const
{
    if (isBackupDue(file))
        return file.save(document);

    return file.exportTo(document, file.filePath());
}

stapik::storage::FileResult MediaRepository::notWritable(const std::filesystem::path& path) const
{
    return stapik::Failure(stapik::storage::FileError{
        .path = path,
        .step = "save",
        .code = std::make_error_code(std::errc::read_only_file_system)
    });
}

std::filesystem::path MediaRepository::archivePath(const int year) const
{
    return m_directory / std::format("{}{}{}", ARCHIVE_PREFIX, stapik::sync::partitionKeyForYear(year), ARCHIVE_SUFFIX);
}

std::optional<int> MediaRepository::archiveYearFrom(const std::string& fileName)
{
    const std::string prefix = ARCHIVE_PREFIX;
    const std::string suffix = ARCHIVE_SUFFIX;
    if (fileName.size() != prefix.size() + ARCHIVE_YEAR_DIGITS + suffix.size())
        return std::nullopt;

    if (!fileName.starts_with(prefix) || !fileName.ends_with(suffix))
        return std::nullopt;

    return stapik::sync::yearFromPartitionKey(std::string_view(fileName).substr(prefix.size(), ARCHIVE_YEAR_DIGITS));
}

std::filesystem::path MediaRepository::directoryPath()
{
    return AppPaths::ensureUserDataDir(stapik::app::AppContext::instance().info().internalName);
}
