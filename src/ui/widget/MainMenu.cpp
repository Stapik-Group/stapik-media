#include "MainMenu.hpp"

MainMenu::MainMenu(Gtk::ApplicationWindow& window, MediaModel& model, const stapik::theme::ThemeRegistry& themes) :
    m_actionHandler(window, model),
    m_menu(window, StandardMenuOptions{
        .cloudItems = true,
        .undoRedoItems = false,
        .aboutItem = true,
        .themes = &themes
    })
{
    m_actionHandler.registerActions();
}

Gtk::PopoverMenuBar& MainMenu::getMenuBar()
{
    return m_menu.menuBar();
}
