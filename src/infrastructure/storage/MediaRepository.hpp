#pragma once

#include "../../core/model/MediaDocument.hpp"

#include "stapik/document/DocumentFile.hpp"

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
    bool save(const MediaDocument& document) const;

    [[nodiscard]] StorageProblem problem() const;
    [[nodiscard]] bool isWritable() const;

private:
    static constexpr int BACKUP_COUNT = 3;
    static constexpr auto FILE_NAME = "media.json";

    stapik::document::DocumentFile<MediaDocument> m_file;
    StorageProblem m_problem = StorageProblem::None;

    [[nodiscard]] static std::filesystem::path filePath();
};
