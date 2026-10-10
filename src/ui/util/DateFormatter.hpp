#pragma once

#include "../../core/model/ReleaseDate.hpp"

#include "stapik/domain/YearMonth.hpp"

#include <string>

namespace DateFormatter
{
    [[nodiscard]] std::string formatReleaseDate(const ReleaseDate& date);
    [[nodiscard]] std::string formatConsumedDate(const stapik::domain::YearMonth& date);
}