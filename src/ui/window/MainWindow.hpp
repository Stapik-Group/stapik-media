#pragma once

#include "../widget/MainMenu.hpp"
#include "../../core/model/MediaModel.hpp"
#include "../../infrastructure/storage/MediaRepository.hpp"
#include "../view/CategorySidebar.hpp"
#include "../view/MediaListView.hpp"
#include "../widget/EntryFilterBar.hpp"
#include "../widget/PaginationBar.hpp"

#include "stapik/sync/SyncStatus.hpp"
#include "stapik/theme/ThemeRegistry.hpp"
#include "stapik/ui/widget/StatusIndicator.hpp"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>

class MainWindow : public Gtk::ApplicationWindow
{
public:
    explicit MainWindow(const stapik::theme::ThemeRegistry& themes);
    ~MainWindow() override = default;
private:
    static constexpr int DEFAULT_WIDTH = 1280;
    static constexpr int DEFAULT_HEIGHT = 800;
    static constexpr int ADD_BUTTON_MARGIN = 8;
    static constexpr auto WINDOW_TITLE = "Stapik Media";
    static constexpr unsigned int CLOSE_TIMEOUT_MS = 5000;
    static constexpr int STATUS_MARGIN = 4;
    static constexpr std::size_t PAGE_SIZE = 10;

    EntryFilterBar m_filterBar;
    PaginationBar m_paginationBar;
    std::size_t m_currentPage = 1;

    MediaRepository m_repository;
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
    StatusIndicator m_statusIndicator;
    bool m_closing = false;
    bool m_closed = false;

    void init();
    void initLayout();
    void reportStorageProblem();
    void initSignals();
    void onSyncStatusChanged(stapik::sync::SyncStatus status);
    bool onCloseRequest();
    void closeNow();
    void onCategorySelected(MediaCategory category);
    void onAddClicked();
    void onEntryEditRequested(std::size_t index);
    void onEntryDeleteRequested(std::size_t index);

    void applyFiltersAndRefresh();
    void onFilterChanged();
    void onPreviousPageRequested();
    void onNextPageRequested();
};
