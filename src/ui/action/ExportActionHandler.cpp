#include "ExportActionHandler.hpp"

#include "../print/EntryPrintout.hpp"
#include "../../core/export/EntryCsv.hpp"

#include "stapik/csv/CsvWriter.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/ui/dialog/FileDialogs.hpp"
#include "stapik/ui/print/PdfExport.hpp"
#include "stapik/ui/print/Printing.hpp"

#include <memory>
#include <utility>

ExportActionHandler::ExportActionHandler(Gtk::ApplicationWindow& window, const MediaModel& model, ListingProvider listing) :
    m_window(window),
    m_model(model),
    m_listing(std::move(listing))
{}

void ExportActionHandler::registerActions()
{
    m_window.add_action("exportCsv", sigc::mem_fun(*this, &ExportActionHandler::onExportCsv));
    m_window.add_action("exportPdf", sigc::mem_fun(*this, &ExportActionHandler::onExportPdf));
    m_window.add_action("print", sigc::mem_fun(*this, &ExportActionHandler::onPrint));
}

void ExportActionHandler::onExportCsv()
{
    const FileDialogOptions options{
        .title = LocaleManager::instance().translate("dialog.export.csv.title"),
        .initialName = CSV_FILE_NAME,
        .lastDirectoryKey = EXPORT_DIRECTORY_KEY
    };

    showSaveFileDialog(
        m_window,
        options,
        [this](const std::filesystem::path& path) { writeCsv(path); },
        [this](const std::string& error) { showFailure(error); });
}

void ExportActionHandler::onExportPdf()
{
    const auto listing = m_listing();
    const auto document = std::make_shared<const EntryPrintout>(listing.heading, listing.entries);

    const FileDialogOptions options{
        .title = LocaleManager::instance().translate("dialog.export.pdf.title"),
        .initialName = PDF_FILE_NAME,
        .lastDirectoryKey = EXPORT_DIRECTORY_KEY
    };

    showSaveFileDialog(
        m_window,
        options,
        [this, document](const std::filesystem::path& path)
        {
            if (const auto exported = stapik::ui::exportToPdf(*document, path); !exported)
                showFailure(exported.error().message());
        },
        [this](const std::string& error) { showFailure(error); });
}

void ExportActionHandler::onPrint()
{
    const auto listing = m_listing();

    stapik::ui::printDocument(
        m_window,
        std::make_shared<const EntryPrintout>(listing.heading, listing.entries),
        [this](const std::string& error)
        {
            const auto& loc = LocaleManager::instance();
            showMessageDialog(m_window, loc.translate("dialog.print.failed.title"), error);
        });
}

void ExportActionHandler::writeCsv(const std::filesystem::path& path) const
{
    const auto& loc = LocaleManager::instance();

    std::vector<std::string> header;
    header.reserve(EntryCsv::HEADER_KEYS.size());
    for (const auto* key : EntryCsv::HEADER_KEYS)
        header.push_back(loc.translate(key));

    stapik::csv::CsvWriter writer(stapik::csv::CsvOptions::forExcel(loc.languageCode()));
    for (const auto& row : EntryCsv::table(m_model.entries(), header))
        writer.addRow(row);

    if (const auto saved = writer.saveTo(path); !saved)
        showFailure(saved.error().message());
}

void ExportActionHandler::showFailure(const std::string& error) const
{
    const auto& loc = LocaleManager::instance();
    showMessageDialog(m_window, loc.translate("dialog.export.failed.title"), loc.translate("dialog.export.failed.text", { { "error", error } }));
}
