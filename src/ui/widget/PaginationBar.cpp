#include "PaginationBar.hpp"

#include <format>


PaginationBar::PaginationBar() : Box(Gtk::Orientation::HORIZONTAL, BAR_SPACING)
{
    initLayout();
}

void PaginationBar::initLayout()
{
    add_css_class("pagination-bar");
    set_halign(Gtk::Align::CENTER);

    m_previousButton.set_label("<");
    m_nextButton.set_label(">");

    m_previousButton.signal_clicked().connect([this] { m_signalPreviousRequested.emit(); });
    m_nextButton.signal_clicked().connect([this] { m_signalNextRequested.emit(); });

    append(m_previousButton);
    append(m_pageLabel);
    append(m_nextButton);

    updatePageLabel();
}

void PaginationBar::setState(const std::size_t currentPage, const std::size_t totalPages)
{
    m_currentPage = currentPage;
    m_totalPages = totalPages;

    m_previousButton.set_sensitive(currentPage > 1);
    m_nextButton.set_sensitive(currentPage < totalPages);

    updatePageLabel();
}

void PaginationBar::updatePageLabel()
{
    m_pageLabel.set_text(std::format("{} / {}", m_currentPage, m_totalPages));
}

sigc::signal<void()>& PaginationBar::signalPreviousRequested()
{
    return m_signalPreviousRequested;
}

sigc::signal<void()>& PaginationBar::signalNextRequested()
{
    return m_signalNextRequested;
}