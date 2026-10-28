#include "ArchiveActionHandler.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/locale/DateFormatter.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/storage/PathText.hpp"
#include "stapik/theme/FontManager.hpp"
#include "stapik/theme/ThemeManager.hpp"
#include "stapik/ui/dialog/ConfirmDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/ui/dialog/FileDialogs.hpp"

#include <chrono>
#include <format>
#include <string>

namespace
{
    constexpr auto ARCHIVE_FILE_EXTENSION = ".zip";

    std::string defaultArchiveName()
    {
        const auto today = stapik::locale::toLocalTime(std::chrono::system_clock::now());
        return std::format("{}-{:%Y-%m-%d}{}", stapik::app::AppContext::instance().info().internalName, today, ARCHIVE_FILE_EXTENSION);
    }

    std::string importFailureText(const stapik::archive::ArchiveError& error)
    {
        const auto& loc = LocaleManager::instance();

        switch (error.kind)
        {
            using enum stapik::archive::ArchiveErrorKind;
            case NotAnArchive: return loc.translate("archive.import.failed.notAnArchive");
            case WrongApplication: return loc.translate("archive.import.failed.wrongApplication");
            case Unsupported: return loc.translate("archive.import.failed.unsupported");
            case Corrupted: return loc.translate("archive.import.failed.corrupted");
            case Io:
            case Cloud:
            default: return loc.translate("archive.import.failed.io", { { "reason", error.detail } });
        }
    }

    void reloadSharedSettings()
    {
        ThemeManager::instance().themeSetting().reload();
        FontManager::instance().fontSetting().reload();
        LocaleManager::instance().languageSetting().reload();
    }
}

ArchiveActionHandler::ArchiveActionHandler(Gtk::ApplicationWindow& window, MediaModel& model, const MediaRepository& repository) :
    m_window(window),
    m_model(model),
    m_repository(repository)
{}

void ArchiveActionHandler::registerActions()
{
    m_window.add_action("exportArchive", sigc::mem_fun(*this, &ArchiveActionHandler::onExportArchive));

    const auto importAction = m_window.add_action("importArchive", sigc::mem_fun(*this, &ArchiveActionHandler::onImportArchive));
    importAction->set_enabled(m_model.isWritable());
}

void ArchiveActionHandler::onExportArchive() const
{
    const auto& loc = LocaleManager::instance();

    const FileDialogOptions options{
        .title = loc.translate("archive.export.title"),
        .initialName = defaultArchiveName(),
        .lastDirectoryKey = ARCHIVE_DIRECTORY_KEY
    };

    showSaveFileDialog(m_window, options, [this](const std::filesystem::path& target) { exportArchiveTo(target); });
}

void ArchiveActionHandler::onImportArchive() const
{
    const auto& loc = LocaleManager::instance();

    const FileDialogOptions options{
        .title = loc.translate("archive.import.title"),
        .initialName = {},
        .lastDirectoryKey = ARCHIVE_DIRECTORY_KEY
    };

    showOpenFileDialog(m_window, options, [this](const std::filesystem::path& source)
    {
        const auto& texts = LocaleManager::instance();

        const ConfirmDialogOptions confirmation{
            .title = texts.translate("dialog.archive.import.title"),
            .message = texts.translate("archive.import.confirm"),
            .confirmLabel = texts.translate("dialog.archive.import.button"),
            .destructive = true
        };

        showConfirmDialog(m_window, confirmation, [this, source] { importArchiveFrom(source); });
    });
}

void ArchiveActionHandler::exportArchiveTo(const std::filesystem::path& target) const
{
    const auto& loc = LocaleManager::instance();

    if (const auto exported = m_repository.exportArchive(m_model.snapshot(), target); !exported)
    {
        showMessageDialog(m_window, loc.translate("archive.export.failed.title"),
            loc.translate("archive.export.failed.text", { { "reason", exported.error().message() } }));
        return;
    }

    showMessageDialog(m_window, loc.translate("archive.export.done.title"),
        loc.translate("archive.export.done.text", { { "path", stapik::storage::pathText(target) } }));
}

void ArchiveActionHandler::importArchiveFrom(const std::filesystem::path& source) const
{
    const auto imported = m_repository.importArchive(source, [this](MediaDocument document)
    {
        return m_model.replaceAll(std::move(document));
    });

    if (!imported)
    {
        const auto& loc = LocaleManager::instance();
        showMessageDialog(m_window, loc.translate("archive.import.failed.title"), importFailureText(imported.error()));
        return;
    }

    reloadSharedSettings();

    const auto& loc = LocaleManager::instance();
    showMessageDialog(m_window, loc.translate("archive.import.done.title"), loc.translate("archive.import.done.text"));
}
