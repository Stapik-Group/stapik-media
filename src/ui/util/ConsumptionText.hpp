#pragma once

#include "../../core/model/MediaCategory.hpp"

#include <string>
#include <string_view>

namespace ConsumptionText
{
    [[nodiscard]] std::string key(std::string_view prefix, MediaCategory category);
}
