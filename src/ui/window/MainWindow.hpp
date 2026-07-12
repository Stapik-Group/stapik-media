#pragma once

#include "../widget/MainMenu.hpp"
#include "../view/CategorySidebar.hpp"
#include "../view/MediaListView.hpp"
#include "../widget/EntryFilterBar.hpp"
#include "../widget/PaginationBar.hpp"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>

class MainWindow : public Gtk::ApplicationWindow
{
public:
    explicit MainWindow();
    ~MainWindow() override = default;
    void initCloud();
private:
    static constexpr int DEFAULT_WIDTH = 1280;
    static constexpr int DEFAULT_HEIGHT = 800;
    static constexpr int ADD_BUTTON_MARGIN = 8;
    static constexpr auto WINDOW_TITLE = "Stapik Media";
    static constexpr auto MEDIA_FILENAME = "media.json";
    static constexpr std::size_t PAGE_SIZE = 10;

    EntryFilterBar m_filterBar;
    PaginationBar m_paginationBar;
    std::size_t m_currentPage = 1;

    MediaModel m_model;

    Gtk::Box m_mainBox;
    Gtk::Box m_contentBox;
    Gtk::Box m_rightBox;
    CategorySidebar m_sidebar;
    Gtk::Button m_addButton;
    Gtk::ScrolledWindow m_listScroller;
    MediaListView m_listView;
    MainMenu m_mainMenu;
    Gtk::Label m_listHeaderLabel;

    static Snapshot loadInitialSnapshot();

    void init();
    void initLayout();
    void initSignals();
    void onCategorySelected(MediaCategory category);
    void onAddClicked();
    void onEntryEditRequested(std::size_t index);
    void onEntryDeleteRequested(std::size_t index);

    void applyFiltersAndRefresh();
    void onFilterChanged();
    void onPreviousPageRequested();
    void onNextPageRequested();
};
