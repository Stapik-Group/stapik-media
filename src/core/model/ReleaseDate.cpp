#include "ReleaseDate.hpp"

#include <charconv>
#include <format>
#include <stdexcept>
#include <string_view>
#include <system_error>

namespace
{
    constexpr std::size_t ISO_DATE_LENGTH = 10;
    constexpr std::size_t YEAR_DIGITS = 4;
    constexpr std::size_t MONTH_OFFSET = 5;
    constexpr std::size_t DAY_OFFSET = 8;
    constexpr std::size_t FIELD_DIGITS = 2;

    bool parseNumber(const std::string_view text, int& value)
    {
        const auto* end = text.data() + text.size();
        const auto [position, error] = std::from_chars(text.data(), end, value);
        return error == std::errc{} && position == end;
    }
}

std::string ReleaseDate::toIsoDateString() const
{
    return std::format("{:04d}-{:02d}-{:02d}",
                       static_cast<int>(date.year()),
                       static_cast<unsigned>(date.month()),
                       static_cast<unsigned>(date.day()));
}

ReleaseDate ReleaseDate::fromIsoDateString(const std::string& str, const DatePrecision precision)
{
    const std::string_view text = str;
    int year = 0;
    int month = 0;
    int day = 0;

    const auto wellFormed = text.size() == ISO_DATE_LENGTH
        && text[YEAR_DIGITS] == '-'
        && text[DAY_OFFSET - 1] == '-'
        && parseNumber(text.substr(0, YEAR_DIGITS), year)
        && parseNumber(text.substr(MONTH_OFFSET, FIELD_DIGITS), month)
        && parseNumber(text.substr(DAY_OFFSET, FIELD_DIGITS), day);

    if (!wellFormed || year < 0 || month < 0 || day < 0)
        throw std::invalid_argument("Invalid release date: " + str);

    const auto date = std::chrono::year{year} / std::chrono::month{static_cast<unsigned>(month)} / std::chrono::day{static_cast<unsigned>(day)};
    if (!date.ok())
        throw std::invalid_argument("Release date does not exist: " + str);

    return ReleaseDate{date, precision};
}
