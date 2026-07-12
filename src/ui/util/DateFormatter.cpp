#include "DateFormatter.hpp"

namespace
{
    std::string pad2(const unsigned value)
    {
        return (value < 10 ? "0" : "") + std::to_string(value);
    }
}

namespace DateFormatter
{
    std::string formatReleaseDate(const ReleaseDate& date)
    {
        const auto year = std::to_string(static_cast<int>(date.date.year()));
        const auto month = pad2(static_cast<unsigned>(date.date.month()));
        const auto day = pad2(static_cast<unsigned>(date.date.day()));

        switch (date.precision)
        {
            case DatePrecision::Day:   return day + "." + month + "." + year;
            case DatePrecision::Month: return month + "." + year;
            case DatePrecision::Year:  return year;
        }
        return year;
    }

    std::string formatConsumedDate(const ConsumedDate& date)
    {
        return pad2(date.month) + "." + std::to_string(date.year);
    }
}