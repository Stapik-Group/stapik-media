#include "MainMenu.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <giomm/menu.h>

namespace
{
    void buildRestoreBackupMenu(Gio::Menu& fileMenu)
    {
        const auto backupSection = Gio::Menu::create();
        backupSection->append(LocaleManager::instance().translate("menu.file.restoreBackup"), "win.restoreBackup");
        fileMenu.append_section(backupSection);
    }
}

MainMenu::MainMenu(Gtk::ApplicationWindow& window, MediaModel& model, const stapik::theme::ThemeRegistry& themes) :
    m_actionHandler(window, model),
    m_menu(window, StandardMenuOptions{
        .cloudItems = true,
        .undoRedoItems = true,
        .aboutItem = true,
        .themes = &themes,
        .undoStack = &model.undoStack()
    })
{
    m_actionHandler.registerActions();
    m_menu.addToMenu(StandardMenu::Target::File, buildRestoreBackupMenu);
}

Gtk::PopoverMenuBar& MainMenu::getMenuBar()
{
    return m_menu.menuBar();
}
