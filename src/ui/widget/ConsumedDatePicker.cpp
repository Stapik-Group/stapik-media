#include "ConsumedDatePicker.hpp"
#include "stapik/locale/LocaleManager.hpp"

ConsumedDatePicker::ConsumedDatePicker() : Box(Gtk::Orientation::VERTICAL, FIELD_SPACING)
{
    initLayout();
}

void ConsumedDatePicker::initLayout()
{
    add_css_class("date-picker");

    const auto& loc = LocaleManager::instance();

    m_monthLabel.set_text(loc.translate("picker.month.label"));
    m_monthSpin.set_range(1, 12);
    m_monthSpin.set_increments(1, 1);

    m_yearLabel.set_text(loc.translate("picker.year.label"));
    m_yearSpin.set_range(MIN_YEAR, MAX_YEAR);
    m_yearSpin.set_increments(1, 10);

    append(m_monthLabel);
    append(m_monthSpin);
    append(m_yearLabel);
    append(m_yearSpin);
}

void ConsumedDatePicker::loadFrom(const ConsumedDate& date)
{
    m_monthSpin.set_value(date.month);
    m_yearSpin.set_value(date.year);
}

ConsumedDate ConsumedDatePicker::toConsumedDate() const
{
    return ConsumedDate{
        static_cast<unsigned>(m_monthSpin.get_value()),
        static_cast<unsigned>(m_yearSpin.get_value())
    };
}