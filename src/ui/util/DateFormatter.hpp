#pragma once

#include "../../core/model/ConsumedDate.hpp"
#include "../../core/model/ReleaseDate.hpp"

#include <string>

namespace DateFormatter
{
    [[nodiscard]] std::string formatReleaseDate(const ReleaseDate& date);
    [[nodiscard]] std::string formatConsumedDate(const ConsumedDate& date);
}