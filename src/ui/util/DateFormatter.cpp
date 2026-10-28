#include "DateFormatter.hpp"

#include "stapik/locale/DateFormatter.hpp"
#include "stapik/locale/LocaleManager.hpp"

#include <format>

namespace DateFormatter
{
    std::string formatReleaseDate(const ReleaseDate& date)
    {
        const auto& localeManager = LocaleManager::instance();
        const auto year = static_cast<int>(date.date.year());

        switch (date.precision)
        {
            case DatePrecision::Day:
                return stapik::locale::formatDate(date.date, localeManager.languageCode());
            case DatePrecision::Month:
                return stapik::domain::YearMonth(year, static_cast<int>(static_cast<unsigned>(date.date.month()))).label(localeManager);
            case DatePrecision::Year:
                return std::format("{}", year);
        }
        return std::format("{}", year);
    }

    std::string formatConsumedDate(const stapik::domain::YearMonth& date)
    {
        return date.label(LocaleManager::instance());
    }
}
