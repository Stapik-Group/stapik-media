#pragma once

#include "../../core/model/MediaModel.hpp"
#include "../../infrastructure/storage/MediaRepository.hpp"

#include <giomm/simpleaction.h>
#include <gtkmm/applicationwindow.h>

#include <filesystem>

class ArchiveActionHandler
{
public:
    ArchiveActionHandler(Gtk::ApplicationWindow& window, MediaModel& model, const MediaRepository& repository);

    void registerActions();

private:
    static constexpr auto ARCHIVE_DIRECTORY_KEY = "archive.lastDirectory";

    Gtk::ApplicationWindow& m_window;
    MediaModel& m_model;
    const MediaRepository& m_repository;

    void onExportArchive() const;
    void onImportArchive() const;
    void exportArchiveTo(const std::filesystem::path& target) const;
    void importArchiveFrom(const std::filesystem::path& source) const;
};
