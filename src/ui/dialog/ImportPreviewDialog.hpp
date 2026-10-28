#pragma once

#include "stapik/ui/dialog/StapikDialog.hpp"

#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>

#include <string>
#include <vector>

class ImportPreviewDialog : public StapikDialog
{
public:
    struct Item
    {
        std::string text;
        bool importable = false;
    };

    ImportPreviewDialog(Window& parent, const std::string& summary, const std::vector<Item>& items, bool anythingToImport);

private:
    static constexpr int DIALOG_WIDTH = 680;
    static constexpr int DIALOG_HEIGHT = 520;
    static constexpr int ROW_MARGIN = 4;
    static constexpr std::size_t PREVIEW_ROW_LIMIT = 500;

    Gtk::Label m_summaryLabel;
    Gtk::ScrolledWindow m_scroller;
    Gtk::ListBox m_listBox;
};
