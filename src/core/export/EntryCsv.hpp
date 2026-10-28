#pragma once

#include "../model/MediaEntry.hpp"

#include "stapik/csv/CsvWriter.hpp"

#include <array>
#include <string>
#include <vector>

namespace EntryCsv
{
    inline constexpr std::array<const char*, 11> HEADER_KEYS = {
        "export.column.category",
        "export.column.title",
        "export.column.releaseDate",
        "export.column.consumed",
        "export.column.creator",
        "export.column.publisher",
        "export.column.genre",
        "export.column.platform",
        "export.column.audiobook",
        "export.column.rating",
        "export.column.note"
    };

    [[nodiscard]] std::vector<stapik::csv::CsvRow> table(const std::vector<MediaEntry>& entries, const std::vector<std::string>& header);
}
