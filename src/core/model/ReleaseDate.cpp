#include "ReleaseDate.hpp"

#include <format>

std::string ReleaseDate::toIsoDateString() const
{
    return std::format("{:04d}-{:02d}-{:02d}",
                       static_cast<int>(date.year()),
                       static_cast<unsigned>(date.month()),
                       static_cast<unsigned>(date.day()));
}

ReleaseDate ReleaseDate::fromIsoDateString(const std::string& str, const DatePrecision precision)
{
    const int year = std::stoi(str.substr(0, 4));
    const auto month = static_cast<unsigned>(std::stoi(str.substr(5, 2)));
    const auto day = static_cast<unsigned>(std::stoi(str.substr(8, 2)));

    return ReleaseDate{
        std::chrono::year{year} / std::chrono::month{month} / std::chrono::day{day},
        precision
    };
}