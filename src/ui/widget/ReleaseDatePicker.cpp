#include "ReleaseDatePicker.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include <gtkmm/stringlist.h>

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

    append(m_precisionLabel);
    append(m_precisionDropDown);
    append(m_dayLabel);
    append(m_daySpin);
    append(m_monthLabel);
    append(m_monthSpin);
    append(m_yearLabel);
    append(m_yearSpin);

    onPrecisionChanged();
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
    m_daySpin.set_value(static_cast<unsigned>(date.date.day()));
    m_monthSpin.set_value(static_cast<unsigned>(date.date.month()));
    m_yearSpin.set_value(static_cast<int>(date.date.year()));
    onPrecisionChanged();
}

ReleaseDate ReleaseDatePicker::toReleaseDate() const
{
    const auto precision = currentPrecision();
    const auto year = std::chrono::year{static_cast<int>(m_yearSpin.get_value())};
    const auto month = precision == DatePrecision::Year
                           ? std::chrono::month{1}
                           : std::chrono::month{static_cast<unsigned>(m_monthSpin.get_value())};
    const auto day = precision == DatePrecision::Day
                         ? std::chrono::day{static_cast<unsigned>(m_daySpin.get_value())}
                         : std::chrono::day{1};

    return ReleaseDate{year / month / day, precision};
}
