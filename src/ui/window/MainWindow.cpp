#include "MainWindow.hpp"

#include "../dialog/MediaEntryDialog.hpp"
#include "../util/EntryFilterService.hpp"

#include "stapik/locale/LocaleManager.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"

#include <glibmm/main.h>

namespace
{
    const char* storageProblemKey(const StorageProblem problem)
    {
        switch (problem)
        {
            using enum StorageProblem;
            case Corrupted: return "storage.problem.corrupted";
            case NewerVersion: return "storage.problem.newerVersion";
            case MigrationFailed: return "storage.problem.migrationFailed";
            case None: break;
        }
        return "";
    }
}

MainWindow::MainWindow(const stapik::theme::ThemeRegistry& themes) :
    m_repository(),
    m_model(m_repository.load(), m_repository),
    m_mainBox(Gtk::Orientation::VERTICAL, 0),
    m_contentBox(Gtk::Orientation::HORIZONTAL, 0),
    m_rightBox(Gtk::Orientation::VERTICAL, 0),
    m_mainMenu(*this, m_model, themes)
{
    init();
    initLayout();
    initSignals();
    reportStorageProblem();
}

void MainWindow::reportStorageProblem()
{
    const auto problem = m_repository.problem();
    if (problem == StorageProblem::None)
        return;

    if (!m_repository.isWritable())
        m_mainBox.set_sensitive(false);

    Glib::signal_idle().connect_once([this, problem]
    {
        const auto& loc = LocaleManager::instance();
        showMessageDialog(*this, loc.translate("storage.problem.header"), loc.translate(storageProblemKey(problem)), Gtk::MessageType::ERROR);
    });
}

void MainWindow::init()
{
    set_title(WINDOW_TITLE);
    set_default_size(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    set_child(m_mainBox);
}

void MainWindow::initLayout()
{
    m_addButton.set_label(LocaleManager::instance().translate("window.button.add"));
    m_addButton.set_margin(ADD_BUTTON_MARGIN);
    m_addButton.add_css_class("add-entry-button");
    m_addButton.set_halign(Gtk::Align::START);

    m_listHeaderLabel.add_css_class("panel-title");
    m_listHeaderLabel.set_halign(Gtk::Align::START);
    m_listHeaderLabel.set_margin(ADD_BUTTON_MARGIN);

    m_listScroller.set_child(m_listView);
    m_listScroller.set_expand(true);
    m_listScroller.add_css_class("media-list-scroller");

    m_rightBox.append(m_listHeaderLabel);
    m_rightBox.append(m_filterBar);
    m_rightBox.append(m_listScroller);
    m_rightBox.append(m_paginationBar);
    m_rightBox.append(m_addButton);

    m_contentBox.append(m_sidebar);
    m_contentBox.append(m_rightBox);

    m_mainBox.append(m_mainMenu.getMenuBar());
    m_statusIndicator.set_margin(STATUS_MARGIN);
    m_statusIndicator.set_visible(m_model.isCloudConnected());
    m_statusIndicator.setStatus(m_model.syncStatus());

    m_mainBox.append(m_contentBox);
    m_mainBox.append(m_statusIndicator);
}

void MainWindow::initSignals()
{
    m_addButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onAddClicked));
    m_sidebar.signalCategorySelected().connect(sigc::mem_fun(*this, &MainWindow::onCategorySelected));
    m_listView.signalEditRequested().connect(sigc::mem_fun(*this, &MainWindow::onEntryEditRequested));
    m_listView.signalDeleteRequested().connect(sigc::mem_fun(*this, &MainWindow::onEntryDeleteRequested));
    m_filterBar.signalFilterChanged().connect(sigc::mem_fun(*this, &MainWindow::onFilterChanged));
    m_paginationBar.signalPreviousRequested().connect(sigc::mem_fun(*this, &MainWindow::onPreviousPageRequested));
    m_paginationBar.signalNextRequested().connect(sigc::mem_fun(*this, &MainWindow::onNextPageRequested));

    m_model.signalSyncStatusChanged().connect(sigc::mem_fun(*this, &MainWindow::onSyncStatusChanged));
    m_model.signalAvailableYearsChanged().connect([this] { m_filterBar.setAvailableYears(m_model.availableYears()); });
    signal_close_request().connect(sigc::mem_fun(*this, &MainWindow::onCloseRequest), false);

    m_model.signalEntriesChanged().connect([this] {
        m_currentPage = 1;
        m_filterBar.setAvailableYears(m_model.availableYears());
        applyFiltersAndRefresh();
    });

    LocaleManager::instance().signalLocaleChanged().connect([this] {
        m_addButton.set_label(LocaleManager::instance().translate("window.button.add"));
        m_sidebar.refreshLabels();
        m_listView.refreshLabels();
        m_filterBar.refreshLabels();
        m_listHeaderLabel.set_text(CategorySidebar::labelFor(m_sidebar.selectedCategory()));
    });

    onCategorySelected(m_sidebar.selectedCategory());
}

void MainWindow::onSyncStatusChanged(const stapik::sync::SyncStatus status)
{
    if (m_closed)
        return;

    m_statusIndicator.set_visible(m_model.isCloudConnected());
    m_statusIndicator.setStatus(status);
}

bool MainWindow::onCloseRequest()
{
    if (m_closing)
        return true;

    m_model.flush();
    if (!m_model.isSyncing())
        return false;

    m_closing = true;
    set_visible(false);

    m_model.signalSyncStatusChanged().connect([this](const stapik::sync::SyncStatus)
    {
        if (!m_model.isSyncing())
            closeNow();
    });
    Glib::signal_timeout().connect_once([this] { closeNow(); }, CLOSE_TIMEOUT_MS);
    return true;
}

void MainWindow::closeNow()
{
    if (m_closed)
        return;

    m_closed = true;
    destroy();
}

void MainWindow::onCategorySelected(const MediaCategory category)
{
    m_currentPage = 1;
    m_filterBar.reset();
    m_filterBar.setAvailableYears(m_model.availableYears());
    m_listHeaderLabel.set_text(CategorySidebar::labelFor(category));
    applyFiltersAndRefresh();
}

void MainWindow::onAddClicked()
{
    auto* dialog = new MediaEntryDialog(*this, m_sidebar.selectedCategory());

    dialog->signal_response().connect([this, dialog](const int responseId)
    {
        if (responseId == Gtk::ResponseType::OK)
        {
            if (const auto result = dialog->getResult(); result.has_value() && !m_model.addEntry(*result))
                showArchiveNotLoaded();
        }
        dialog->hide();
    });

    dialog->signal_hide().connect([dialog] { delete dialog; });
    dialog->show();
}

void MainWindow::onEntryEditRequested(const std::size_t index)
{
    const auto& entries = m_model.entries();
    if (index >= entries.size())
    {
        stapik::log::warning("MainWindow::onEntryEditRequested: invalid index {}", index);
        return;
    }

    const auto id = entries.at(index).id;
    auto* dialog = new MediaEntryDialog(*this, entries.at(index));

    dialog->signal_response().connect([this, dialog, id](const int responseId)
    {
        if (responseId == Gtk::ResponseType::OK)
        {
            if (const auto result = dialog->getResult(); result.has_value() && !m_model.updateEntry(id, *result))
                showArchiveNotLoaded();
        }
        dialog->hide();
    });

    dialog->signal_hide().connect([dialog] { delete dialog; });
    dialog->show();
}

void MainWindow::onEntryDeleteRequested(const std::size_t index)
{
    const auto& entries = m_model.entries();
    if (index >= entries.size())
        return;

    m_model.removeEntry(entries.at(index).id);
}

void MainWindow::showArchiveNotLoaded()
{
    const auto& loc = LocaleManager::instance();
    showMessageDialog(*this, loc.translate("partition.notLoaded.header"), loc.translate("partition.notLoaded.message"), Gtk::MessageType::WARNING);
}

void MainWindow::onFilterChanged()
{
    if (const auto year = m_filterBar.selectedYear(); year.has_value())
        m_model.requestYear(static_cast<int>(*year));

    m_currentPage = 1;
    applyFiltersAndRefresh();
}

void MainWindow::onPreviousPageRequested()
{
    if (m_currentPage > 1)
        --m_currentPage;
    applyFiltersAndRefresh();
}

void MainWindow::onNextPageRequested()
{
    ++m_currentPage;
    applyFiltersAndRefresh();
}

void MainWindow::applyFiltersAndRefresh()
{
    const EntryFilter filter{
        m_sidebar.selectedCategory(),
        m_filterBar.selectedMonth(),
        m_filterBar.selectedYear()
    };

    const auto matchingIndices = EntryFilterService::filterIndices(m_model.entries(), filter);

    const auto totalPages = matchingIndices.empty()
        ? std::size_t{1}
    : (matchingIndices.size() + PAGE_SIZE - 1) / PAGE_SIZE;

    if (m_currentPage > totalPages)
        m_currentPage = totalPages;

    const auto startIndex = (m_currentPage - 1) * PAGE_SIZE;
    const auto endIndex = std::min(startIndex + PAGE_SIZE, matchingIndices.size());

    const std::vector pageIndices(
        matchingIndices.begin() + static_cast<std::ptrdiff_t>(startIndex),
        matchingIndices.begin() + static_cast<std::ptrdiff_t>(endIndex));

    m_listView.refresh(m_model.entries(), pageIndices);
    m_paginationBar.setState(m_currentPage, totalPages);
}
