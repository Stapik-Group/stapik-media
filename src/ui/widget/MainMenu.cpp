#include "MainMenu.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/update/UpdateChecker.hpp"

#include <giomm/menu.h>

namespace
{
    void buildDisconnectMenu(Gio::Menu& fileMenu)
    {
        const auto disconnectSection = Gio::Menu::create();
        disconnectSection->append(LocaleManager::instance().translate("menu.file.disconnect"), "win.disconnect");
        fileMenu.append_section(disconnectSection);
    }

    void buildImportMenu(Gio::Menu& fileMenu)
    {
        const auto importSection = Gio::Menu::create();
        importSection->append(LocaleManager::instance().translate("menu.file.importCsv"), "win.importCsv");
        fileMenu.append_section(importSection);
    }

    void buildExportMenu(Gio::Menu& fileMenu)
    {
        const auto& loc = LocaleManager::instance();
        const auto exportSection = Gio::Menu::create();
        exportSection->append(loc.translate("menu.file.exportCsv"), "win.exportCsv");
        exportSection->append(loc.translate("menu.file.print"), "win.print");
        exportSection->append(loc.translate("menu.file.exportPdf"), "win.exportPdf");
        fileMenu.append_section(exportSection);
    }

    void buildRestoreBackupMenu(Gio::Menu& fileMenu)
    {
        const auto backupSection = Gio::Menu::create();
        backupSection->append(LocaleManager::instance().translate("menu.file.restoreBackup"), "win.restoreBackup");
        fileMenu.append_section(backupSection);
    }
}

MainMenu::MainMenu(Gtk::ApplicationWindow& window, MediaModel& model, const stapik::theme::ThemeRegistry& themes) :
    m_actionHandler(window, model),
    m_updateCheckSetting(stapik::app::AppContext::instance().settings(), stapik::update::CHECK_ENABLED_SETTING, true),
    m_menu(window, StandardMenuOptions{
        .cloudItems = true,
        .undoRedoItems = true,
        .aboutItem = true,
        .tutorialDirectory = AppPaths::resourcesDir() / "tutorial",
        .themes = &themes,
        .undoStack = &model.undoStack()
    })
{
    m_actionHandler.registerActions();
    m_menu.settings().addBool("settings.checkForUpdates", m_updateCheckSetting);
    m_menu.addToMenu(StandardMenu::Target::File, buildDisconnectMenu);
    m_menu.addToMenu(StandardMenu::Target::File, buildImportMenu);
    m_menu.addToMenu(StandardMenu::Target::File, buildExportMenu);
    m_menu.addToMenu(StandardMenu::Target::File, buildRestoreBackupMenu);
}

Gtk::PopoverMenuBar& MainMenu::getMenuBar()
{
    return m_menu.menuBar();
}
