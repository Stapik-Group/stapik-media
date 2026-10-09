#pragma once

#include "../../core/model/MediaDocument.hpp"

#include "stapik/document/DocumentFile.hpp"

#include <filesystem>
#include <map>
#include <string>

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

    [[nodiscard]] MediaDocument load();
    [[nodiscard]] std::map<int, MediaDocument> loadArchives();
    bool save(const MediaDocument& document) const;
    bool saveArchive(int year, const MediaDocument& document) const;

    [[nodiscard]] StorageProblem problem() const;
    [[nodiscard]] bool isWritable() const;

private:
    static constexpr int BACKUP_COUNT = 3;
    static constexpr auto FILE_NAME = "media.json";
    static constexpr auto ARCHIVE_PREFIX = "media-";
    static constexpr auto ARCHIVE_SUFFIX = ".json";
    static constexpr std::size_t ARCHIVE_YEAR_DIGITS = 4;

    std::filesystem::path m_directory;
    stapik::document::DocumentFile<MediaDocument> m_file;
    StorageProblem m_problem = StorageProblem::None;

    void raiseProblem(StorageProblem problem);
    [[nodiscard]] stapik::document::DocumentFile<MediaDocument> archiveFile(const std::filesystem::path& path) const;
    [[nodiscard]] std::filesystem::path archivePath(int year) const;
    [[nodiscard]] static std::optional<int> archiveYearFrom(const std::string& fileName);
    [[nodiscard]] static std::filesystem::path directoryPath();
};
