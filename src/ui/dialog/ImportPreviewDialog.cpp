#include "ImportPreviewDialog.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <gtkmm/label.h>

#include <algorithm>

ImportPreviewDialog::ImportPreviewDialog(Window& parent, const std::string& summary, const std::vector<Item>& items, const bool anythingToImport) :
    StapikDialog(parent, LocaleManager::instance().translate("import.preview.title"))
{
    const auto& loc = LocaleManager::instance();

    m_summaryLabel.set_text(summary);
    m_summaryLabel.set_halign(Gtk::Align::START);
    m_summaryLabel.set_wrap(true);

    const auto shown = std::min(items.size(), PREVIEW_ROW_LIMIT);
    for (std::size_t index = 0; index < shown; ++index)
    {
        auto* label = Gtk::make_managed<Gtk::Label>(items[index].text);
        label->set_halign(Gtk::Align::START);
        label->set_margin(ROW_MARGIN);
        if (!items[index].importable)
            label->add_css_class("dim-label");

        m_listBox.append(*label);
    }

    if (items.size() > shown)
    {
        auto* label = Gtk::make_managed<Gtk::Label>(loc.translate("import.preview.more", { { "count", std::to_string(items.size() - shown) } }));
        label->set_halign(Gtk::Align::START);
        label->set_margin(ROW_MARGIN);
        label->add_css_class("dim-label");
        m_listBox.append(*label);
    }

    m_listBox.set_selection_mode(Gtk::SelectionMode::NONE);
    m_scroller.set_child(m_listBox);
    m_scroller.set_vexpand(true);

    contentBox().append(m_summaryLabel);
    contentBox().append(m_scroller);

    addCancelButton();
    auto* importButton = addOkButton(loc.translate("import.button"));
    importButton->set_sensitive(anythingToImport);

    set_default_size(DIALOG_WIDTH, DIALOG_HEIGHT);
}
