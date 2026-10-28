#pragma once

#include "MediaEntry.hpp"

#include <string>

namespace EntryIdentity
{
    [[nodiscard]] std::string titleKey(const std::string& title);
    [[nodiscard]] std::string workKey(const MediaEntry& entry);
    [[nodiscard]] bool sameWork(const MediaEntry& left, const MediaEntry& right);
}
