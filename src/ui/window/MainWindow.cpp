#include "MainWindow.hpp"

#include "../../infrastructure/storage/Storage.hpp"
#include "../dialog/MediaEntryDialog.hpp"
#include "../util/EntryFilterService.hpp"

#include "stapik/storage/CloudStorageConfigStorage.hpp"
#include "stapik/locale/LocaleManager.hpp"

MainWindow::MainWindow() :
    m_model(loadInitialSnapshot()),
    m_mainBox(Gtk::Orientation::VERTICAL, 0),
    m_contentBox(Gtk::Orientation::HORIZONTAL, 0),
    m_rightBox(Gtk::Orientation::VERTICAL, 0),
    m_mainMenu(*this, m_model)
{
    init();
    initLayout();
    initSignals();
    initCloud();
}

Snapshot MainWindow::loadInitialSnapshot()
{
    return Storage::load();
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
    m_mainBox.append(m_contentBox);
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

    m_model.signalEntriesChanged().connect([this] {
        m_currentPage = 1;
        m_filterBar.setAvailableYears(EntryFilterService::distinctConsumedYears(m_model.entries()));
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

void MainWindow::initCloud()
{
    const auto config = CloudStorageConfigStorage::load("stapikmedia");
    if (!config.has_value())
        return;

    m_model.setCloudClient(
        std::make_unique<CloudStorageClient>(config.value(), MEDIA_FILENAME));
}

void MainWindow::onCategorySelected(const MediaCategory category)
{
    m_currentPage = 1;
    m_filterBar.reset();
    m_filterBar.setAvailableYears(EntryFilterService::distinctConsumedYears(m_model.entries()));
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
            if (const auto result = dialog->getResult(); result.has_value())
                m_model.addEntry(*result);
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
        g_warning("MainWindow::onEntryEditRequested: invalid index %zu", index);
        return;
    }

    auto* dialog = new MediaEntryDialog(*this, entries.at(index));

    dialog->signal_response().connect([this, dialog, index](const int responseId)
    {
        if (responseId == Gtk::ResponseType::OK)
        {
            if (const auto result = dialog->getResult(); result.has_value())
                m_model.updateEntry(index, *result);
        }
        dialog->hide();
    });

    dialog->signal_hide().connect([dialog] { delete dialog; });
    dialog->show();
}

void MainWindow::onEntryDeleteRequested(const std::size_t index)
{
    m_model.removeEntry(index);
}

void MainWindow::onFilterChanged()
{
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
