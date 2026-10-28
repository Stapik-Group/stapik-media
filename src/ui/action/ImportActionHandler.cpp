#include "ImportActionHandler.hpp"

#include "../dialog/ImportPreviewDialog.hpp"
#include "../view/CategorySidebar.hpp"
#include "../../core/export/CsvReader.hpp"

#include "stapik/locale/LocaleManager.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/ui/dialog/FileDialogs.hpp"

#include <format>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

namespace
{
    const char* problemKey(const EntryCsvImport::RowProblem problem)
    {
        switch (problem)
        {
            using enum EntryCsvImport::RowProblem;
            case MissingColumns: return "import.problem.missingColumns";
            case UnknownCategory: return "import.problem.unknownCategory";
            case EmptyTitle: return "import.problem.emptyTitle";
            case InvalidReleaseDate: return "import.problem.invalidReleaseDate";
            case InvalidConsumed: return "import.problem.invalidConsumed";
            case InvalidAudiobook: return "import.problem.invalidAudiobook";
        }
        return "import.problem.missingColumns";
    }

    const char* statusKey(const MediaModel::ImportVerdict verdict)
    {
        switch (verdict)
        {
            using enum MediaModel::ImportVerdict;
            case Add: return "import.status.new";
            case Duplicate: return "import.status.duplicate";
            case ArchiveNotLoaded: return "import.status.archiveNotLoaded";
        }
        return "import.status.new";
    }
}

ImportActionHandler::ImportActionHandler(Gtk::ApplicationWindow& window, MediaModel& model) :
    m_window(window),
    m_model(model)
{}

void ImportActionHandler::registerActions()
{
    m_window.add_action("importCsv", sigc::mem_fun(*this, &ImportActionHandler::onImportCsv));
}

void ImportActionHandler::onImportCsv()
{
    const auto& loc = LocaleManager::instance();

    if (!m_model.isWritable())
    {
        showMessageDialog(m_window, loc.translate("storage.problem.header"), loc.translate("storage.readOnly.message"));
        return;
    }

    const FileDialogOptions options{
        .title = loc.translate("import.dialog.title"),
        .initialName = {},
        .lastDirectoryKey = IMPORT_DIRECTORY_KEY
    };

    showOpenFileDialog(
        m_window,
        options,
        [this](const std::filesystem::path& path) { importFrom(path); },
        [this](const std::string& error) { showMessageDialog(m_window, LocaleManager::instance().translate("import.failed.title"), error); });
}

void ImportActionHandler::importFrom(const std::filesystem::path& path)
{
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    if (error)
    {
        showFailure("import.failed.unreadable");
        return;
    }

    if (size > MAX_FILE_BYTES)
    {
        showFailure("import.failed.tooLarge");
        return;
    }

    std::ifstream file(path, std::ios::binary);
    const std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (!file.is_open() || file.bad())
    {
        showFailure("import.failed.unreadable");
        return;
    }

    const auto table = CsvReader::parse(content);
    if (!table.has_value())
    {
        showFailure("import.failed.malformed");
        return;
    }

    const auto rows = EntryCsvImport::parse(*table);
    if (rows.empty())
    {
        showFailure("import.failed.empty");
        return;
    }

    showPreview(rows);
}

void ImportActionHandler::showPreview(const std::vector<EntryCsvImport::Row>& rows)
{
    const auto& loc = LocaleManager::instance();

    std::vector<MediaEntry> candidates;
    for (const auto& row : rows)
    {
        if (row.entry.has_value())
            candidates.push_back(*row.entry);
    }

    const auto verdicts = m_model.previewImport(candidates);

    std::vector<ImportPreviewDialog::Item> items;
    items.reserve(rows.size());

    std::size_t added = 0;
    std::size_t duplicates = 0;
    std::size_t unavailable = 0;
    std::size_t invalid = 0;
    std::size_t candidateIndex = 0;

    for (const auto& row : rows)
    {
        if (!row.entry.has_value())
        {
            ++invalid;
            items.push_back({
                loc.translate("import.row.invalid", {
                    { "reason", loc.translate(problemKey(*row.problem)) },
                    { "row", std::to_string(row.number) },
                    { "title", row.title }
                }),
                false
            });
            continue;
        }

        const auto verdict = verdicts[candidateIndex++];
        added += verdict == MediaModel::ImportVerdict::Add;
        duplicates += verdict == MediaModel::ImportVerdict::Duplicate;
        unavailable += verdict == MediaModel::ImportVerdict::ArchiveNotLoaded;

        const auto& entry = *row.entry;
        items.push_back({
            std::format("{}  {} – {} ({})",
                loc.translate(statusKey(verdict)),
                CategorySidebar::labelFor(entry.category),
                entry.title,
                static_cast<int>(entry.releaseDate.date.year())),
            verdict == MediaModel::ImportVerdict::Add
        });
    }

    const auto summary = loc.translate("import.preview.summary", {
        { "added", std::to_string(added) },
        { "duplicates", std::to_string(duplicates) },
        { "invalid", std::to_string(invalid) },
        { "unavailable", std::to_string(unavailable) }
    });

    auto* dialog = new ImportPreviewDialog(m_window, summary, items, added > 0);
    auto shared = std::make_shared<std::vector<MediaEntry>>(std::move(candidates));

    dialog->signal_response().connect([this, dialog, shared](const int responseId)
    {
        dialog->hide();
        if (responseId != Gtk::ResponseType::OK)
            return;

        const auto count = m_model.importEntries(*shared);
        const auto& texts = LocaleManager::instance();
        showMessageDialog(m_window, texts.translate("import.done.title"), texts.translate("import.done.message", { { "count", std::to_string(count) } }));
    });

    deleteOnHide(dialog);
    dialog->show();
}

void ImportActionHandler::showFailure(const std::string_view messageKey) const
{
    const auto& loc = LocaleManager::instance();
    showMessageDialog(m_window, loc.translate("import.failed.title"), loc.translate(messageKey));
}
