#pragma once

#include "stapik/csv/CsvWriter.hpp"

#include <optional>
#include <string_view>
#include <vector>

namespace CsvReader
{
    [[nodiscard]] std::optional<std::vector<stapik::csv::CsvRow>> parse(std::string_view content);
}
