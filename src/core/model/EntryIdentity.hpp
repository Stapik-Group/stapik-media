#pragma once

#include "MediaEntry.hpp"

#include <string>

namespace EntryIdentity
{
    [[nodiscard]] std::string titleKey(const std::string& title);
    [[nodiscard]] bool sameWork(const MediaEntry& left, const MediaEntry& right);
}
