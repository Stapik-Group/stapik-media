#include "MainWindow.hpp"

#include "../dialog/MediaEntryDialog.hpp"
#include "../util/EntryFilterService.hpp"
#include "../util/Pagination.hpp"
#include "../../core/model/MediaCategoryKey.hpp"

#include "stapik/app/AppContext.hpp"

#include "stapik/locale/DateFormatter.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"

#include <glibmm/main.h>

#include <format>

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
    m_exportActions(*this, m_model, [this] { return currentListing(); }),
    m_mainBox(Gtk::Orientation::VERTICAL, 0),
    m_contentBox(Gtk::Orientation::HORIZONTAL, 0),
    m_rightBox(Gtk::Orientation::VERTICAL, 0),
    m_mainMenu(*this, m_model, themes)
{
    init();
    initLayout();
    initSignals();
    m_exportActions.registerActions();
    reportStorageProblem();
    reportUnreadableEntries();
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
        showMessageDialog(*this, loc.translate("storage.problem.header"), loc.translate(storageProblemKey(problem)));
    });
}

void MainWindow::reportUnreadableEntries()
{
    const auto count = m_model.unreadableEntryCount();
    if (count == 0)
        return;

    Glib::signal_idle().connect_once([this, count]
    {
        const auto& loc = LocaleManager::instance();
        showMessageDialog(*this, loc.translate("storage.unreadable.header"), loc.translate("storage.unreadable.message", { { "count", std::to_string(count) } }));
    });
}

void MainWindow::init()
{
    set_title(stapik::app::AppContext::instance().info().displayName);
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

    const auto storedCategory = categoryFromKey(stapik::app::AppContext::instance().settings().get(SETTING_CATEGORY, std::string{}));
    if (storedCategory.has_value())
        m_sidebar.selectCategory(*storedCategory);

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
    m_model.signalSaveFailed().connect(sigc::mem_fun(*this, &MainWindow::onSaveFailed));
    m_model.signalCloudConflict().connect(sigc::mem_fun(*this, &MainWindow::onCloudConflict));
    m_model.signalCloudApiKeyExpiring().connect(sigc::mem_fun(*this, &MainWindow::onApiKeyExpiring));
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
    m_statusIndicator.set_visible(m_model.isCloudConnected());
    m_statusIndicator.setStatus(status);
}

bool MainWindow::onCloseRequest()
{
    saveWindowState();
    m_model.flushCloud();
    return false;
}

void MainWindow::saveWindowState() const
{
    auto& settings = stapik::app::AppContext::instance().settings();
    settings.set(SETTING_CATEGORY, categoryToKey(m_sidebar.selectedCategory()));

    if (const auto saved = settings.save(); !saved)
        stapik::log::warning("Cannot persist the selected category: {}", saved.error().message());
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
    openEntryDialog(
        new MediaEntryDialog(*this, m_sidebar.selectedCategory()),
        [this](MediaEntry entry) { return m_model.addEntry(std::move(entry)); });
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
    openEntryDialog(
        new MediaEntryDialog(*this, entries.at(index)),
        [this, id](MediaEntry entry) { return m_model.updateEntry(id, std::move(entry)); });
}

void MainWindow::openEntryDialog(MediaEntryDialog* dialog, std::function<MediaModel::EntryResult(MediaEntry)> submit)
{
    dialog->signal_response().connect([this, dialog, submit = std::move(submit)](const int responseId)
    {
        if (responseId == Gtk::ResponseType::OK)
        {
            if (const auto entry = dialog->getResult(); entry.has_value())
            {
                const auto outcome = submit(*entry);
                if (!outcome)
                {
                    reportRejectedEntry(*dialog, outcome.error(), *entry);
                    if (outcome.error() == MediaModel::EntryError::Duplicate)
                        return;
                }
            }
        }
        dialog->hide();
    });

    deleteOnHide(dialog);
    dialog->show();
}

void MainWindow::reportRejectedEntry(Gtk::Window& dialog, const MediaModel::EntryError error, const MediaEntry& entry)
{
    const auto& loc = LocaleManager::instance();

    switch (error)
    {
        using enum MediaModel::EntryError;
        case ArchiveNotLoaded:
            showArchiveNotLoaded();
            break;
        case Duplicate:
            showMessageDialog(dialog, loc.translate("dialog.entry.duplicate.header"), loc.translate("dialog.entry.duplicate.message", { { "title", entry.title }, { "year", std::format("{}", static_cast<int>(entry.releaseDate.date.year())) } }));
            break;
        case UnknownEntry:
            showMessageDialog(*this, loc.translate("dialog.entry.missing.header"), loc.translate("dialog.entry.missing.message"));
            break;
    }
}

void MainWindow::onEntryDeleteRequested(const std::size_t index)
{
    const auto& entries = m_model.entries();
    if (index >= entries.size())
        return;

    m_model.removeEntry(entries.at(index).id);
}

void MainWindow::onSaveFailed(const std::string& message)
{
    const auto& loc = LocaleManager::instance();
    showMessageDialog(*this, loc.translate("dialog.save.failed.title"), loc.translate("dialog.save.failed.text", { { "error", message } }));
}

void MainWindow::onCloudConflict()
{
    const auto& loc = LocaleManager::instance();
    showMessageDialog(*this, loc.translate("cloud.conflict.header"), loc.translate("cloud.conflict.secondary"));
}

void MainWindow::onApiKeyExpiring(const MediaModel::TimePoint expiresAt)
{
    const auto& loc = LocaleManager::instance();
    const auto date = stapik::locale::formatDate(stapik::locale::toLocalTime(expiresAt), loc.languageCode());

    showMessageDialog(*this, loc.translate("cloud.apiKeyExpiring.header"), loc.translate("cloud.apiKeyExpiring", { { "date", date } }));
}

void MainWindow::showArchiveNotLoaded()
{
    const auto& loc = LocaleManager::instance();
    showMessageDialog(*this, loc.translate("partition.notLoaded.header"), loc.translate("partition.notLoaded.message"));
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

EntryFilter MainWindow::currentFilter() const
{
    return EntryFilter{
        m_sidebar.selectedCategory(),
        m_filterBar.selectedMonth(),
        m_filterBar.selectedYear(),
        m_filterBar.searchQuery()
    };
}

ExportActionHandler::Listing MainWindow::currentListing() const
{
    const auto& entries = m_model.entries();
    const auto matchingIndices = EntryFilterService::filterIndices(entries, currentFilter());

    ExportActionHandler::Listing listing{
        .heading = std::format("{} \u2013 {}", get_title().raw(), CategorySidebar::labelFor(m_sidebar.selectedCategory())),
        .entries = {}
    };
    listing.entries.reserve(matchingIndices.size());
    for (const auto index : matchingIndices)
        listing.entries.push_back(entries[index]);

    return listing;
}

void MainWindow::applyFiltersAndRefresh()
{
    const auto matchingIndices = EntryFilterService::filterIndices(m_model.entries(), currentFilter());
    const auto slice = Pagination::sliceFor(matchingIndices.size(), m_currentPage, PAGE_SIZE);
    m_currentPage = slice.page;

    const std::vector pageIndices(
        matchingIndices.begin() + static_cast<std::ptrdiff_t>(slice.first),
        matchingIndices.begin() + static_cast<std::ptrdiff_t>(slice.last));

    m_listView.refresh(m_model.entries(), pageIndices);
    m_paginationBar.setState(slice.page, slice.totalPages);
}
