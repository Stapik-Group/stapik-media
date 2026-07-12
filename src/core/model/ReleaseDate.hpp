#pragma once

#include <chrono>
#include <string>

enum class DatePrecision { Day, Month, Year };

struct ReleaseDate {
    std::chrono::year_month_day date;
    DatePrecision precision;

    [[nodiscard]] std::string toIsoDateString() const;
    static ReleaseDate fromIsoDateString(const std::string& str, DatePrecision precision);
};