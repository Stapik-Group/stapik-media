#include "ConsumptionText.hpp"

#include <format>

namespace
{
    std::string_view verbOf(const MediaCategory category)
    {
        switch (category)
        {
            using enum MediaCategory;
            case Movie:
            case Series:
            case Cartoon: return "watched";
            case Book: return "read";
            case Album: return "listened";
            case Game: return "completed";
        }
        return "watched";
    }
}

namespace ConsumptionText
{
    std::string key(const std::string_view prefix, const MediaCategory category)
    {
        return std::format("{}.{}", prefix, verbOf(category));
    }
}
