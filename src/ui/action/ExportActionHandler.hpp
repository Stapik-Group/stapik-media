#pragma once

#include "../../core/model/MediaModel.hpp"

#include <gtkmm/applicationwindow.h>

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

class ExportActionHandler
{
public:
    struct Listing
    {
        std::string heading;
        std::vector<MediaEntry> entries;
    };

    using ListingProvider = std::function<Listing()>;

    ExportActionHandler(Gtk::ApplicationWindow& window, const MediaModel& model, ListingProvider listing);

    void registerActions();

private:
    static constexpr auto EXPORT_DIRECTORY_KEY = "export.lastDirectory";
    static constexpr auto CSV_FILE_NAME = "stapik-media.csv";
    static constexpr auto PDF_FILE_NAME = "stapik-media.pdf";

    Gtk::ApplicationWindow& m_window;
    const MediaModel& m_model;
    ListingProvider m_listing;

    void onExportCsv();
    void onExportPdf();
    void onPrint();
    void writeCsv(const std::filesystem::path& path) const;
    void showFailure(const std::string& error) const;
};
