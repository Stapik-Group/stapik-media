#pragma once

#include "../action/MenuActionHandler.hpp"

#include "stapik/theme/ThemeRegistry.hpp"
#include "stapik/ui/menu/StandardMenu.hpp"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/popovermenubar.h>

class MainMenu
{
public:
    MainMenu(Gtk::ApplicationWindow& window, MediaModel& model, const stapik::theme::ThemeRegistry& themes);
    ~MainMenu() = default;
    Gtk::PopoverMenuBar& getMenuBar();

private:
    MenuActionHandler m_actionHandler;
    StandardMenu m_menu;
};
