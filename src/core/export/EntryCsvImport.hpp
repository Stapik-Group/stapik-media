#pragma once

#include "../model/MediaEntry.hpp"

#include "stapik/csv/CsvWriter.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace EntryCsvImport
{
    enum class RowProblem
    {
        MissingColumns,
        UnknownCategory,
        EmptyTitle,
        InvalidReleaseDate,
        InvalidConsumed,
        InvalidAudiobook
    };

    struct Row
    {
        std::size_t number = 0;
        std::string title;
        std::optional<MediaEntry> entry;
        std::optional<RowProblem> problem;
    };

    [[nodiscard]] std::vector<Row> parse(const std::vector<stapik::csv::CsvRow>& table);
}
