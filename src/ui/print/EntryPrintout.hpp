#pragma once

#include "../../core/model/MediaEntry.hpp"

#include "stapik/ui/print/PrintableDocument.hpp"

#include <string>
#include <vector>

class EntryPrintout final : public stapik::ui::IPrintableDocument
{
public:
    EntryPrintout(std::string heading, const std::vector<MediaEntry>& entries);

    [[nodiscard]] std::string title() const override;
    [[nodiscard]] int pageCount(const stapik::ui::PageSize& pageSize) const override;
    void drawPage(const Cairo::RefPtr<Cairo::Context>& context, int pageIndex, const stapik::ui::PageSize& pageSize) const override;

private:
    struct Row
    {
        std::string title;
        std::string details;
        std::string dates;
    };

    static constexpr double MARGIN = 48.0;
    static constexpr double HEADING_SIZE = 16.0;
    static constexpr double HEADER_HEIGHT = 40.0;
    static constexpr double FOOTER_HEIGHT = 28.0;
    static constexpr double TITLE_SIZE = 11.0;
    static constexpr double DETAIL_SIZE = 9.0;
    static constexpr double LINE_GAP = 3.0;
    static constexpr double ROW_HEIGHT = 44.0;
    static constexpr double DETAIL_GRAY = 0.35;

    std::string m_heading;
    std::vector<Row> m_rows;

    [[nodiscard]] std::size_t rowsPerPage(const stapik::ui::PageSize& pageSize) const;
};
