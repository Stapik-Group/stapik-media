#pragma once

#include "../../core/model/MediaDocument.hpp"

#include "stapik/document/DocumentFile.hpp"
#include "stapik/storage/FileError.hpp"
#include "stapik/time/Clock.hpp"

#include <chrono>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

enum class StorageProblem
{
    None,
    Corrupted,
    NewerVersion,
    MigrationFailed
};

class MediaRepository
{
public:
    MediaRepository();
    explicit MediaRepository(std::filesystem::path directory, const stapik::time::IClock& clock = stapik::time::systemClock());

    [[nodiscard]] MediaDocument load();
    [[nodiscard]] std::map<int, MediaDocument> loadArchives();
    [[nodiscard]] stapik::storage::FileResult save(const MediaDocument& document) const;
    [[nodiscard]] stapik::storage::FileResult saveArchive(int year, const MediaDocument& document) const;
    [[nodiscard]] std::vector<stapik::document::BackupInfo> listBackups() const;
    [[nodiscard]] stapik::document::LoadResult<MediaDocument> loadBackup(const stapik::document::BackupInfo& backup) const;

    [[nodiscard]] StorageProblem problem() const;
    [[nodiscard]] bool isWritable() const;

private:
    static constexpr int BACKUP_COUNT = 5;
    static constexpr std::chrono::minutes BACKUP_INTERVAL{ 5 };
    static constexpr auto FILE_NAME = "media.json";
    static constexpr auto ARCHIVE_PREFIX = "media-";
    static constexpr auto ARCHIVE_SUFFIX = ".json";
    static constexpr std::size_t ARCHIVE_YEAR_DIGITS = 4;

    std::filesystem::path m_directory;
    stapik::document::DocumentFile<MediaDocument> m_file;
    const stapik::time::IClock& m_clock;
    StorageProblem m_problem = StorageProblem::None;

    void raiseProblem(StorageProblem problem);
    [[nodiscard]] bool isBackupDue(const stapik::document::DocumentFile<MediaDocument>& file) const;
    [[nodiscard]] stapik::storage::FileResult write(const stapik::document::DocumentFile<MediaDocument>& file, const MediaDocument& document) const;
    [[nodiscard]] stapik::storage::FileResult notWritable(const std::filesystem::path& path) const;
    [[nodiscard]] stapik::document::DocumentFile<MediaDocument> archiveFile(const std::filesystem::path& path) const;
    [[nodiscard]] std::filesystem::path archivePath(int year) const;
    [[nodiscard]] static std::optional<int> archiveYearFrom(const std::string& fileName);
    [[nodiscard]] static std::filesystem::path directoryPath();
};
