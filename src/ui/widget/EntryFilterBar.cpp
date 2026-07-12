#include "EntryFilterBar.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <gtkmm/stringlist.h>

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

    m_yearDropDown.property_selected().signal_changed().connect(sigc::mem_fun(*this, &EntryFilterBar::onYearChanged));
    m_monthDropDown.property_selected().signal_changed().connect(sigc::mem_fun(*this, &EntryFilterBar::onMonthChanged));

    rebuildMonthOptions();
}

void EntryFilterBar::setAvailableYears(const std::set<unsigned>& years)
{
    const auto& loc = LocaleManager::instance();

    std::vector<Glib::ustring> labels{ loc.translate("filter.year.all") };
    m_yearValues.clear();
    m_yearValues.push_back(0);

    for (const auto year : years)
    {
        labels.emplace_back(std::to_string(year));
        m_yearValues.push_back(year);
    }

    const auto model = Gtk::StringList::create(labels);
    m_yearDropDown.set_model(model);
    m_yearDropDown.set_selected(ALL_YEARS_INDEX);
}

void EntryFilterBar::rebuildMonthOptions()
{
    const auto& loc = LocaleManager::instance();

    std::vector<Glib::ustring> labels{ loc.translate("filter.month.all") };
    for (unsigned m = 1; m <= 12; ++m)
        labels.emplace_back(std::to_string(m));

    const auto model = Gtk::StringList::create(labels);
    m_monthDropDown.set_model(model);
    m_monthDropDown.set_selected(ALL_MONTHS_INDEX);
}

void EntryFilterBar::onYearChanged()
{
    const bool yearSelected = m_yearDropDown.get_selected() != ALL_YEARS_INDEX;

    m_monthDropDown.set_sensitive(yearSelected);
    if (!yearSelected)
        m_monthDropDown.set_selected(ALL_MONTHS_INDEX);

    m_signalFilterChanged.emit();
}

void EntryFilterBar::onMonthChanged() const
{
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

void EntryFilterBar::reset()
{
    m_yearDropDown.set_selected(ALL_YEARS_INDEX);
    m_monthDropDown.set_selected(ALL_MONTHS_INDEX);
    m_monthDropDown.set_sensitive(false);
}

void EntryFilterBar::refreshLabels()
{
    const auto& loc = LocaleManager::instance();
    m_yearLabel.set_text(loc.translate("filter.year.label"));
    m_monthLabel.set_text(loc.translate("filter.month.label"));
}

sigc::signal<void()> & EntryFilterBar::signalFilterChanged()
{
    return m_signalFilterChanged;
}
