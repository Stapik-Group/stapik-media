#pragma once

#include "stapik/domain/YearMonth.hpp"

#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/label.h>

class ConsumedDatePicker : public Gtk::Box
{
public:
    ConsumedDatePicker();

    void loadFrom(const stapik::domain::YearMonth& date);
    [[nodiscard]] stapik::domain::YearMonth toYearMonth() const;

private:
    static constexpr int FIELD_SPACING = 6;
    static constexpr int MIN_YEAR = 1900;
    static constexpr int MAX_YEAR = 2100;

    Gtk::Label m_monthLabel;
    Gtk::SpinButton m_monthSpin;
    Gtk::Label m_yearLabel;
    Gtk::SpinButton m_yearSpin;

    void initLayout();
};
