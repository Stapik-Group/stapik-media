#include "MediaRepository.hpp"

#include "MediaDocumentSchema.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/sync/PartitionKey.hpp"

#include <format>
#include <string_view>

MediaRepository::MediaRepository() :
    m_directory(directoryPath()),
    m_file(m_directory / FILE_NAME, createMediaSchemaMigrator())
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
            if (result.migrated && !m_file.save(*result.document))
                stapik::log::warning("Cannot save the migrated media document");

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
                if (result.migrated && !file.save(*result.document))
                    stapik::log::warning("Cannot save the migrated media archive {}", *year);

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

bool MediaRepository::save(const MediaDocument& document) const
{
    if (!isWritable())
        return false;

    if (!m_file.save(document))
    {
        stapik::log::warning("Cannot save the media document");
        return false;
    }

    return true;
}

bool MediaRepository::saveArchive(const int year, const MediaDocument& document) const
{
    if (!isWritable())
        return false;

    if (!archiveFile(archivePath(year)).save(document))
    {
        stapik::log::warning("Cannot save the media archive {}", year);
        return false;
    }

    return true;
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
    stapik::document::DocumentFile<MediaDocument> file(path, createMediaSchemaMigrator());
    file.setBackupCount(BACKUP_COUNT);
    return file;
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
