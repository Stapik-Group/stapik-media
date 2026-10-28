#include "ReleaseDatePicker.hpp"
#include "stapik/domain/YearMonth.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include <gtkmm/stringlist.h>

#include <algorithm>

ReleaseDatePicker::ReleaseDatePicker() : Box(Gtk::Orientation::VERTICAL, FIELD_SPACING)
{
    initLayout();
}

void ReleaseDatePicker::initLayout()
{
    add_css_class("date-picker");

    const auto &loc = LocaleManager::instance();

    const auto precisionOptions = Gtk::StringList::create({
        loc.translate("picker.precision.day"),
        loc.translate("picker.precision.month"),
        loc.translate("picker.precision.year")
    });

    m_precisionDropDown.set_model(precisionOptions);
    m_precisionDropDown.set_selected(0);
    m_precisionDropDown.property_selected().signal_changed().connect(sigc::mem_fun(*this, &ReleaseDatePicker::onPrecisionChanged));

    m_precisionLabel.set_text(loc.translate("picker.precision.label"));
    m_precisionLabel.set_halign(Gtk::Align::START);

    m_dayLabel.set_text(loc.translate("picker.day.label"));
    m_daySpin.set_range(1, 31);
    m_daySpin.set_increments(1, 1);
    m_daySpin.set_value(DEFAULT_DAY);

    m_monthLabel.set_text(loc.translate("picker.month.label"));
    m_monthSpin.set_range(1, 12);
    m_monthSpin.set_increments(1, 1);
    m_monthSpin.set_value(DEFAULT_MONTH);

    m_yearLabel.set_text(loc.translate("picker.year.label"));
    m_yearSpin.set_range(MIN_YEAR, MAX_YEAR);
    m_yearSpin.set_increments(1, 10);
    m_yearSpin.set_value(stapik::domain::YearMonth::current().year());

    m_monthSpin.signal_value_changed().connect(sigc::mem_fun(*this, &ReleaseDatePicker::updateDayRange));
    m_yearSpin.signal_value_changed().connect(sigc::mem_fun(*this, &ReleaseDatePicker::updateDayRange));

    append(m_precisionLabel);
    append(m_precisionDropDown);
    append(m_dayLabel);
    append(m_daySpin);
    append(m_monthLabel);
    append(m_monthSpin);
    append(m_yearLabel);
    append(m_yearSpin);

    onPrecisionChanged();
    updateDayRange();
}

void ReleaseDatePicker::updateDayRange()
{
    m_daySpin.set_range(1, ReleaseDate::daysInMonth(m_yearSpin.get_value_as_int(), static_cast<unsigned>(m_monthSpin.get_value_as_int())));
}

void ReleaseDatePicker::onPrecisionChanged()
{
    using enum DatePrecision;
    const auto precision = currentPrecision();
    m_dayLabel.set_visible(precision == Day);
    m_daySpin.set_visible(precision == Day);
    m_monthLabel.set_visible(precision == Day || precision == Month);
    m_monthSpin.set_visible(precision == Day || precision == Month);
}

DatePrecision ReleaseDatePicker::currentPrecision() const
{
    switch (m_precisionDropDown.get_selected())
    {
        using enum DatePrecision;
        case 0: return Day;
        case 1: return Month;
        default: return Year;
    }
}

void ReleaseDatePicker::loadFrom(const ReleaseDate &date)
{
    m_precisionDropDown.set_selected(static_cast<guint>(date.precision));
    m_yearSpin.set_value(static_cast<int>(date.date.year()));
    m_monthSpin.set_value(static_cast<unsigned>(date.date.month()));
    updateDayRange();
    m_daySpin.set_value(static_cast<unsigned>(date.date.day()));
    onPrecisionChanged();
}

ReleaseDate ReleaseDatePicker::toReleaseDate() const
{
    const auto precision = currentPrecision();
    const auto yearValue = m_yearSpin.get_value_as_int();
    const auto monthValue = precision == DatePrecision::Year ? 1U : static_cast<unsigned>(m_monthSpin.get_value_as_int());
    const auto dayValue = precision == DatePrecision::Day
                              ? std::min(m_daySpin.get_value_as_int(), ReleaseDate::daysInMonth(yearValue, monthValue))
                              : 1;

    const auto year = std::chrono::year{yearValue};
    const auto month = std::chrono::month{monthValue};
    const auto day = std::chrono::day{static_cast<unsigned>(dayValue)};

    return ReleaseDate{year / month / day, precision};
}
