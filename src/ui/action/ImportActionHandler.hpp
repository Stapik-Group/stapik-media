#pragma once

#include "../../core/export/EntryCsvImport.hpp"
#include "../../core/model/MediaModel.hpp"

#include <gtkmm/applicationwindow.h>

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

class ImportActionHandler
{
public:
    ImportActionHandler(Gtk::ApplicationWindow& window, MediaModel& model);

    void registerActions();

private:
    static constexpr auto IMPORT_DIRECTORY_KEY = "import.lastDirectory";
    static constexpr std::uintmax_t MAX_FILE_BYTES = 16U * 1024U * 1024U;

    Gtk::ApplicationWindow& m_window;
    MediaModel& m_model;

    void onImportCsv();
    void importFrom(const std::filesystem::path& path);
    void showPreview(const std::vector<EntryCsvImport::Row>& rows);
    void showFailure(std::string_view messageKey) const;
};
