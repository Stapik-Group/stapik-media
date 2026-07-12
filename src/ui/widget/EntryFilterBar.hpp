#pragma once

#include <gtkmm/box.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/label.h>
#include <sigc++/signal.h>
#include <vector>
#include <set>

class EntryFilterBar : public Gtk::Box
{
public:
    EntryFilterBar();

    void setAvailableYears(const std::set<unsigned>& years);
    void reset();
    void refreshLabels();

    [[nodiscard]] std::optional<unsigned> selectedYear() const;
    [[nodiscard]] std::optional<unsigned> selectedMonth() const;

    sigc::signal<void()>& signalFilterChanged();

private:
    static constexpr int BAR_SPACING = 8;
    static constexpr guint ALL_YEARS_INDEX = 0;
    static constexpr guint ALL_MONTHS_INDEX = 0;

    Gtk::Label m_monthLabel;
    Gtk::DropDown m_monthDropDown;
    Gtk::Label m_yearLabel;
    Gtk::DropDown m_yearDropDown;

    std::vector<unsigned> m_yearValues;

    sigc::signal<void()> m_signalFilterChanged;

    void initLayout();
    void rebuildMonthOptions();
    void onYearChanged();
    void onMonthChanged() const;
};
