#pragma once

#include "../../core/model/ReleaseDate.hpp"

#include <gtkmm/box.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/label.h>

class ReleaseDatePicker : public Gtk::Box
{
public:
    ReleaseDatePicker();

    void loadFrom(const ReleaseDate& date);
    [[nodiscard]] ReleaseDate toReleaseDate() const;

private:
    static constexpr int FIELD_SPACING = 6;
    static constexpr int MIN_YEAR = 1900;
    static constexpr int MAX_YEAR = 2100;
    static constexpr int DEFAULT_DAY = 1;
    static constexpr int DEFAULT_MONTH = 1;

    Gtk::Label m_precisionLabel;
    Gtk::DropDown m_precisionDropDown;

    Gtk::Label m_dayLabel;
    Gtk::SpinButton m_daySpin;
    Gtk::Label m_monthLabel;
    Gtk::SpinButton m_monthSpin;
    Gtk::Label m_yearLabel;
    Gtk::SpinButton m_yearSpin;

    void initLayout();
    void onPrecisionChanged();
    [[nodiscard]] DatePrecision currentPrecision() const;
};
