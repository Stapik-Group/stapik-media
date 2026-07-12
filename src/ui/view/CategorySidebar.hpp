#pragma once

#include "../../core/model/MediaCategory.hpp"

#include <gtkmm/box.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>
#include <gtkmm/label.h>
#include <sigc++/signal.h>
#include <array>

class CategorySidebar : public Gtk::Box
{
public:
    CategorySidebar();
    [[nodiscard]] MediaCategory selectedCategory() const;
    void refreshLabels();
    [[nodiscard]] static std::string labelFor(MediaCategory category);
    sigc::signal<void(MediaCategory)>& signalCategorySelected();
private:
    static constexpr int SIDEBAR_SPACING = 0;
    static constexpr int SIDEBAR_WIDTH = 160;

    static constexpr std::array<MediaCategory, 6> ALL_CATEGORIES = {
        MediaCategory::Movie, MediaCategory::Series, MediaCategory::Cartoon,
        MediaCategory::Book, MediaCategory::Album, MediaCategory::Game
    };

    Gtk::ListBox m_listBox;
    std::array<Gtk::ListBoxRow, ALL_CATEGORIES.size()> m_rows;
    std::array<Gtk::Label, ALL_CATEGORIES.size()> m_labels;

    sigc::signal<void(MediaCategory)> m_signalCategorySelected;

    void initLayout();
    [[nodiscard]] static std::string translationKeyFor(MediaCategory category);
    void onRowSelected(const Gtk::ListBoxRow* row) const;
};