#include "CsvReader.hpp"

namespace
{
    constexpr std::string_view BYTE_ORDER_MARK = "\xEF\xBB\xBF";
    constexpr char QUOTE = '"';

    char detectSeparator(const std::string_view content)
    {
        std::size_t commas = 0;
        std::size_t semicolons = 0;
        auto quoted = false;

        for (const auto character : content)
        {
            if (character == QUOTE)
                quoted = !quoted;
            else if (!quoted && (character == '\n' || character == '\r'))
                break;
            else if (!quoted && character == ',')
                ++commas;
            else if (!quoted && character == ';')
                ++semicolons;
        }

        return semicolons > commas ? ';' : ',';
    }
}

namespace CsvReader
{
    std::optional<std::vector<stapik::csv::CsvRow>> parse(std::string_view content)
    {
        if (content.starts_with(BYTE_ORDER_MARK))
            content.remove_prefix(BYTE_ORDER_MARK.size());

        const auto separator = detectSeparator(content);

        std::vector<stapik::csv::CsvRow> rows;
        stapik::csv::CsvRow row;
        std::string field;
        auto quoted = false;
        auto recordHasContent = false;

        const auto endRecord = [&]
        {
            if (recordHasContent)
            {
                row.push_back(std::move(field));
                rows.push_back(std::move(row));
            }

            row.clear();
            field.clear();
            recordHasContent = false;
        };

        for (std::size_t position = 0; position < content.size(); ++position)
        {
            const auto character = content[position];

            if (quoted)
            {
                if (character != QUOTE)
                    field += character;
                else if (position + 1 < content.size() && content[position + 1] == QUOTE)
                {
                    field += QUOTE;
                    ++position;
                }
                else
                    quoted = false;
                continue;
            }

            if (character == QUOTE && field.empty())
            {
                quoted = true;
                recordHasContent = true;
            }
            else if (character == separator)
            {
                row.push_back(std::move(field));
                field.clear();
                recordHasContent = true;
            }
            else if (character == '\r' || character == '\n')
            {
                if (character == '\r' && position + 1 < content.size() && content[position + 1] == '\n')
                    ++position;
                endRecord();
            }
            else
            {
                field += character;
                recordHasContent = true;
            }
        }

        if (quoted)
            return std::nullopt;

        endRecord();
        return rows;
    }
}
