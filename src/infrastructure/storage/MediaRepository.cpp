#include "MediaRepository.hpp"

#include "MediaDocumentSchema.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/AppPaths.hpp"

MediaRepository::MediaRepository() :
    m_file(filePath(), createMediaSchemaMigrator())
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
            m_problem = StorageProblem::Corrupted;
            return {};
        case NewerVersion:
            m_problem = StorageProblem::NewerVersion;
            return {};
        case MigrationFailed:
            m_problem = StorageProblem::MigrationFailed;
            return {};
    }

    return {};
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

StorageProblem MediaRepository::problem() const
{
    return m_problem;
}

bool MediaRepository::isWritable() const
{
    return m_problem != StorageProblem::NewerVersion && m_problem != StorageProblem::MigrationFailed;
}

std::filesystem::path MediaRepository::filePath()
{
    return AppPaths::ensureUserDataDir(stapik::app::AppContext::instance().info().internalName) / FILE_NAME;
}
