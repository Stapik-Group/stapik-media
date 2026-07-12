#include "CategorySidebar.hpp"
#include "stapik/locale/LocaleManager.hpp"

CategorySidebar::CategorySidebar() : Box(Gtk::Orientation::VERTICAL, SIDEBAR_SPACING)
{
    initLayout();
}

void CategorySidebar::initLayout()
{
    set_size_request(SIDEBAR_WIDTH, -1);
    add_css_class("category-sidebar");

    for (std::size_t i = 0; i < ALL_CATEGORIES.size(); ++i)
    {
        m_labels[i].set_text(LocaleManager::instance().translate(translationKeyFor(ALL_CATEGORIES[i])));
        m_labels[i].set_halign(Gtk::Align::START);
        m_labels[i].set_margin(SIDEBAR_SPACING);
        m_rows[i].set_child(m_labels[i]);
        m_listBox.append(m_rows[i]);
    }

    m_listBox.select_row(m_rows[0]);
    m_listBox.signal_row_selected().connect(sigc::mem_fun(*this, &CategorySidebar::onRowSelected));

    append(m_listBox);
}

void CategorySidebar::onRowSelected(const Gtk::ListBoxRow *row) const
{
    if (row == nullptr)
        return;

    const auto index = static_cast<std::size_t>(row->get_index());
    if (index >= ALL_CATEGORIES.size())
    {
        g_warning("CategorySidebar::onRowSelected: index out of range: %zu", index);
        return;
    }

    m_signalCategorySelected.emit(ALL_CATEGORIES[index]);
}

MediaCategory CategorySidebar::selectedCategory() const
{
    const auto *row = m_listBox.get_selected_row();
    if (row == nullptr)
        return ALL_CATEGORIES[0];

    const auto index = static_cast<std::size_t>(row->get_index());
    return ALL_CATEGORIES.at(index);
}

void CategorySidebar::refreshLabels()
{
    const auto &loc = LocaleManager::instance();
    for (std::size_t i = 0; i < ALL_CATEGORIES.size(); ++i)
        m_labels[i].set_text(loc.translate(translationKeyFor(ALL_CATEGORIES[i])));
}

std::string CategorySidebar::translationKeyFor(const MediaCategory category)
{
    switch (category)
    {
        using enum MediaCategory;
        case Movie: return "sidebar.category.movie";
        case Series: return "sidebar.category.series";
        case Cartoon: return "sidebar.category.cartoon";
        case Book: return "sidebar.category.book";
        case Album: return "sidebar.category.album";
        case Game: return "sidebar.category.game";
    }
    throw std::invalid_argument("Unknown MediaCategory in translationKeyFor");
}

sigc::signal<void(MediaCategory)> & CategorySidebar::signalCategorySelected()
{
    return m_signalCategorySelected;
}

std::string CategorySidebar::labelFor(const MediaCategory category)
{
    return LocaleManager::instance().translate(translationKeyFor(category));
}
