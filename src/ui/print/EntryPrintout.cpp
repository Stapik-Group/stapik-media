#include "EntryPrintout.hpp"

#include "../util/EntryText.hpp"

#include <cairo.h>
#include <glibmm/ustring.h>

#include <algorithm>
#include <format>

namespace
{
    std::string fitText(cairo_t* context, const std::string& text, const double maxWidth)
    {
        cairo_text_extents_t extents;
        cairo_text_extents(context, text.c_str(), &extents);
        if (extents.x_advance <= maxWidth)
            return text;

        Glib::ustring shown(text);
        while (!shown.empty())
        {
            shown.erase(shown.size() - 1, 1);
            const Glib::ustring candidate = shown + "\u2026";
            cairo_text_extents(context, candidate.c_str(), &extents);
            if (extents.x_advance <= maxWidth)
                return candidate.raw();
        }

        return {};
    }

    void drawText(cairo_t* context, const std::string& text, const double x, const double y, const double size, const bool bold, const double gray, const double maxWidth)
    {
        cairo_select_font_face(context, "Sans", CAIRO_FONT_SLANT_NORMAL, bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(context, size);
        cairo_set_source_rgb(context, gray, gray, gray);
        cairo_move_to(context, x, y);
        cairo_show_text(context, fitText(context, text, maxWidth).c_str());
    }
}

EntryPrintout::EntryPrintout(std::string heading, const std::vector<MediaEntry>& entries) :
    m_heading(std::move(heading))
{
    m_rows.reserve(entries.size());
    for (const auto& entry : entries)
        m_rows.push_back({ .title = entry.title, .details = EntryText::detailsLine(entry), .dates = EntryText::datesLine(entry) });
}

std::string EntryPrintout::title() const
{
    return m_heading;
}

std::size_t EntryPrintout::rowsPerPage(const stapik::ui::PageSize& pageSize) const
{
    const auto usableHeight = pageSize.height - 2 * MARGIN - HEADER_HEIGHT - FOOTER_HEIGHT;
    return static_cast<std::size_t>(std::max(usableHeight / ROW_HEIGHT, 1.0));
}

int EntryPrintout::pageCount(const stapik::ui::PageSize& pageSize) const
{
    const auto perPage = rowsPerPage(pageSize);
    return static_cast<int>(std::max<std::size_t>((m_rows.size() + perPage - 1) / perPage, 1));
}

void EntryPrintout::drawPage(const Cairo::RefPtr<Cairo::Context>& context, const int pageIndex, const stapik::ui::PageSize& pageSize) const
{
    auto* cairo = context->cobj();
    const auto contentWidth = pageSize.width - 2 * MARGIN;

    drawText(cairo, m_heading, MARGIN, MARGIN + HEADING_SIZE, HEADING_SIZE, true, 0.0, contentWidth);

    cairo_set_source_rgb(cairo, 0.0, 0.0, 0.0);
    cairo_set_line_width(cairo, 0.5);
    cairo_move_to(cairo, MARGIN, MARGIN + HEADER_HEIGHT - LINE_GAP * 4);
    cairo_line_to(cairo, pageSize.width - MARGIN, MARGIN + HEADER_HEIGHT - LINE_GAP * 4);
    cairo_stroke(cairo);

    const auto perPage = rowsPerPage(pageSize);
    const auto first = std::min(static_cast<std::size_t>(pageIndex) * perPage, m_rows.size());
    const auto last = std::min(first + perPage, m_rows.size());

    for (auto index = first; index < last; ++index)
    {
        const auto& row = m_rows[index];
        const auto top = MARGIN + HEADER_HEIGHT + static_cast<double>(index - first) * ROW_HEIGHT;
        const auto titleBaseline = top + TITLE_SIZE;
        const auto detailsBaseline = titleBaseline + LINE_GAP + DETAIL_SIZE;
        const auto datesBaseline = detailsBaseline + LINE_GAP + DETAIL_SIZE;

        drawText(cairo, row.title, MARGIN, titleBaseline, TITLE_SIZE, true, 0.0, contentWidth);
        drawText(cairo, row.details, MARGIN, detailsBaseline, DETAIL_SIZE, false, DETAIL_GRAY, contentWidth);
        drawText(cairo, row.dates, MARGIN, datesBaseline, DETAIL_SIZE, false, DETAIL_GRAY, contentWidth);
    }

    const auto footer = std::format("{} / {}", pageIndex + 1, pageCount(pageSize));
    cairo_select_font_face(cairo, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cairo, DETAIL_SIZE);
    cairo_text_extents_t extents;
    cairo_text_extents(cairo, footer.c_str(), &extents);
    drawText(cairo, footer, pageSize.width - MARGIN - extents.x_advance, pageSize.height - MARGIN, DETAIL_SIZE, false, DETAIL_GRAY, contentWidth);
}
