#include "EntryFilterBar.hpp"

#include "../util/ConsumptionText.hpp"

#include "stapik/domain/YearMonth.hpp"
#include "stapik/locale/LocaleManager.hpp"

#include <gtkmm/stringlist.h>

#include <algorithm>

namespace
{
    class ScopedFlag
    {
    public:
        explicit ScopedFlag(bool& flag) :
            m_flag(flag),
            m_previous(flag)
        {
            m_flag = true;
        }

        ~ScopedFlag()
        {
            m_flag = m_previous;
        }

        ScopedFlag(const ScopedFlag&) = delete;
        ScopedFlag& operator=(const ScopedFlag&) = delete;

    private:
        bool& m_flag;
        bool m_previous;
    };
}

EntryFilterBar::EntryFilterBar() : Box(Gtk::Orientation::HORIZONTAL, BAR_SPACING)
{
    initLayout();
}

void EntryFilterBar::initLayout()
{
    add_css_class("entry-filter-bar");

    const auto& loc = LocaleManager::instance();

    m_yearLabel.set_text(loc.translate("filter.year.label"));
    m_monthLabel.set_text(loc.translate("filter.month.label"));

    append(m_yearLabel);
    append(m_yearDropDown);
    append(m_monthLabel);
    append(m_monthDropDown);

    m_searchEntry.set_placeholder_text(loc.translate("filter.search.placeholder"));
    m_searchEntry.set_hexpand(true);
    m_searchEntry.signalQueryChanged().connect([this] { m_signalFilterChanged.emit(); });
    append(m_searchEntry);

    m_sortLabel.set_text(loc.translate("filter.sort.label"));
    append(m_sortLabel);
    append(m_sortDropDown);
    append(m_sortDirectionButton);

    m_yearDropDown.property_selected().signal_changed().connect(sigc::mem_fun(*this, &EntryFilterBar::onYearChanged));
    m_monthDropDown.property_selected().signal_changed().connect(sigc::mem_fun(*this, &EntryFilterBar::onMonthChanged));

    m_sortDropDown.property_selected().signal_changed().connect(sigc::mem_fun(*this, &EntryFilterBar::onSortFieldChanged));
    m_sortDirectionButton.signal_clicked().connect(sigc::mem_fun(*this, &EntryFilterBar::onSortDirectionClicked));

    rebuildMonthOptions();
    rebuildSortOptions();
    refreshSortDirection();
}

void EntryFilterBar::setAvailableYears(const std::set<unsigned>& years)
{
    m_lastAvailableYears = years;

    const auto& loc = LocaleManager::instance();
    const auto previousSelection = selectedYear();

    std::vector<Glib::ustring> labels{ loc.translate("filter.year.all") };
    m_yearValues.clear();
    m_yearValues.push_back(0);

    for (const auto year : years)
    {
        labels.emplace_back(std::to_string(year));
        m_yearValues.push_back(year);
    }

    {
        const ScopedFlag updating(m_updating);

        m_yearDropDown.set_model(Gtk::StringList::create(labels));

        const auto it = previousSelection.has_value() ? std::ranges::find(m_yearValues, previousSelection.value()) : m_yearValues.end();
        m_yearDropDown.set_selected(it != m_yearValues.end()
            ? static_cast<guint>(std::distance(m_yearValues.begin(), it))
            : ALL_YEARS_INDEX);

        const auto yearSelected = selectedYear().has_value();
        m_monthDropDown.set_sensitive(yearSelected);
        if (!yearSelected)
            m_monthDropDown.set_selected(ALL_MONTHS_INDEX);
    }

    if (selectedYear() != previousSelection)
        m_signalFilterChanged.emit();
}

void EntryFilterBar::rebuildMonthOptions()
{
    const auto& loc = LocaleManager::instance();

    std::vector<Glib::ustring> labels{ loc.translate("filter.month.all") };
    for (int month = 1; month <= stapik::domain::MONTHS_PER_YEAR; ++month)
        labels.emplace_back(stapik::domain::YearMonth(stapik::domain::MIN_YEAR_MONTH_YEAR, month).monthName(loc));

    const ScopedFlag updating(m_updating);

    m_monthDropDown.set_model(Gtk::StringList::create(labels));
    m_monthDropDown.set_selected(ALL_MONTHS_INDEX);
}

void EntryFilterBar::rebuildSortOptions()
{
    const auto& loc = LocaleManager::instance();

    const std::vector<Glib::ustring> labels{
        loc.translate("filter.sort.title"),
        loc.translate("filter.sort.releaseDate"),
        loc.translate(ConsumptionText::key("filter.sort.consumed", m_category))
    };

    const ScopedFlag updating(m_updating);

    m_sortDropDown.set_model(Gtk::StringList::create(labels));

    const auto it = std::ranges::find(SORT_FIELDS, m_sort.field);
    m_sortDropDown.set_selected(static_cast<guint>(std::distance(SORT_FIELDS.begin(), it)));
}

void EntryFilterBar::refreshSortDirection()
{
    const auto& loc = LocaleManager::instance();
    const auto ascending = m_sort.direction == SortDirection::Ascending;

    m_sortDirectionButton.set_label(ascending ? "\u2191" : "\u2193");
    m_sortDirectionButton.set_tooltip_text(loc.translate(ascending ? "filter.sort.ascending" : "filter.sort.descending"));
}

void EntryFilterBar::onSortFieldChanged()
{
    if (m_updating)
        return;

    const auto index = m_sortDropDown.get_selected();
    if (index >= SORT_FIELDS.size())
        return;

    m_sort.field = SORT_FIELDS[index];
    m_signalFilterChanged.emit();
}

void EntryFilterBar::onSortDirectionClicked()
{
    m_sort.direction = m_sort.direction == SortDirection::Ascending ? SortDirection::Descending : SortDirection::Ascending;
    refreshSortDirection();
    m_signalFilterChanged.emit();
}

EntrySort EntryFilterBar::selectedSort() const
{
    return m_sort;
}

void EntryFilterBar::setCategory(const MediaCategory category)
{
    if (m_category == category)
        return;

    m_category = category;
    rebuildSortOptions();
}

void EntryFilterBar::setSort(const EntrySort& sort)
{
    m_sort = sort;
    rebuildSortOptions();
    refreshSortDirection();
}

void EntryFilterBar::onYearChanged()
{
    if (m_updating)
        return;

    const bool yearSelected = m_yearDropDown.get_selected() != ALL_YEARS_INDEX;

    m_monthDropDown.set_sensitive(yearSelected);
    if (!yearSelected)
        m_monthDropDown.set_selected(ALL_MONTHS_INDEX);

    m_signalFilterChanged.emit();
}

void EntryFilterBar::onMonthChanged() const
{
    if (m_updating)
        return;

    m_signalFilterChanged.emit();
}

std::optional<unsigned> EntryFilterBar::selectedYear() const
{
    const auto index = m_yearDropDown.get_selected();
    if (index == ALL_YEARS_INDEX || index == GTK_INVALID_LIST_POSITION)
        return std::nullopt;

    return m_yearValues.at(index);
}

std::optional<unsigned> EntryFilterBar::selectedMonth() const
{
    const auto index = m_monthDropDown.get_selected();
    if (index == ALL_MONTHS_INDEX || index == GTK_INVALID_LIST_POSITION)
        return std::nullopt;

    return index;
}

std::string EntryFilterBar::searchQuery() const
{
    return m_searchEntry.query();
}

void EntryFilterBar::reset()
{
    m_searchEntry.set_text("");
    m_yearDropDown.set_selected(ALL_YEARS_INDEX);
    m_monthDropDown.set_selected(ALL_MONTHS_INDEX);
    m_monthDropDown.set_sensitive(false);
}

void EntryFilterBar::refreshLabels()
{
    const auto& loc = LocaleManager::instance();
    m_yearLabel.set_text(loc.translate("filter.year.label"));
    m_monthLabel.set_text(loc.translate("filter.month.label"));
    m_searchEntry.set_placeholder_text(loc.translate("filter.search.placeholder"));
    m_sortLabel.set_text(loc.translate("filter.sort.label"));
    rebuildSortOptions();
    refreshSortDirection();

    const auto previousMonth = selectedMonth();

    setAvailableYears(m_lastAvailableYears);
    rebuildMonthOptions();

    if (previousMonth.has_value())
    {
        const ScopedFlag updating(m_updating);
        m_monthDropDown.set_selected(previousMonth.value());
    }
}

sigc::signal<void()> & EntryFilterBar::signalFilterChanged()
{
    return m_signalFilterChanged;
}
