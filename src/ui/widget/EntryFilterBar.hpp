#pragma once

#include "stapik/ui/widget/SearchEntry.hpp"

#include <gtkmm/box.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/label.h>
#include <sigc++/signal.h>
#include <optional>
#include <set>
#include <string>
#include <vector>

class EntryFilterBar : public Gtk::Box
{
public:
    EntryFilterBar();

    void setAvailableYears(const std::set<unsigned>& years);
    void reset();
    void refreshLabels();

    [[nodiscard]] std::optional<unsigned> selectedYear() const;
    [[nodiscard]] std::optional<unsigned> selectedMonth() const;
    [[nodiscard]] std::string searchQuery() const;

    sigc::signal<void()>& signalFilterChanged();

private:
    static constexpr int BAR_SPACING = 8;
    static constexpr guint ALL_YEARS_INDEX = 0;
    static constexpr guint ALL_MONTHS_INDEX = 0;

    Gtk::Label m_monthLabel;
    Gtk::DropDown m_monthDropDown;
    Gtk::Label m_yearLabel;
    Gtk::DropDown m_yearDropDown;
    SearchEntry m_searchEntry;

    std::vector<unsigned> m_yearValues;
    std::set<unsigned> m_lastAvailableYears;

    bool m_updating = false;

    sigc::signal<void()> m_signalFilterChanged;

    void initLayout();
    void rebuildMonthOptions();
    void onYearChanged();
    void onMonthChanged() const;
};
