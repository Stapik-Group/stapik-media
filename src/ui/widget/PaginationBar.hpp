#pragma once

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <sigc++/signal.h>

class PaginationBar : public Gtk::Box
{
public:
    PaginationBar();
    void setState(std::size_t currentPage, std::size_t totalPages);

    sigc::signal<void()>& signalPreviousRequested();
    sigc::signal<void()>& signalNextRequested();
private:
    static constexpr int BAR_SPACING = 8;

    Gtk::Button m_previousButton;
    Gtk::Label m_pageLabel;
    Gtk::Button m_nextButton;

    std::size_t m_currentPage = 1;
    std::size_t m_totalPages = 1;

    sigc::signal<void()> m_signalPreviousRequested;
    sigc::signal<void()> m_signalNextRequested;

    void initLayout();
    void updatePageLabel();
};