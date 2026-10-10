#pragma once

#include "details/DetailsFormWidget.hpp"

#include "stapik/ui/dialog/StapikDialog.hpp"

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <optional>
#include <memory>

#include "../widget/ConsumedDatePicker.hpp"
#include "../widget/ReleaseDatePicker.hpp"

class MediaEntryDialog : public StapikDialog
{
public:
    MediaEntryDialog(Window& parent, MediaCategory category);
    MediaEntryDialog(Window& parent, const MediaEntry& existing);

    [[nodiscard]] std::optional<MediaEntry> getResult() const;

private:
    static constexpr int DEFAULT_WIDTH = 420;
    static constexpr int DETAILS_SPACING = 8;

    MediaCategory m_category;
    std::string m_id;
    Gtk::Label m_titleLabel;
    Gtk::Entry m_titleEntry;
    ReleaseDatePicker m_releaseDatePicker;
    ConsumedDatePicker m_consumedDatePicker;
    Gtk::Box m_detailsContainer;
    std::unique_ptr<DetailsFormWidget> m_detailsForm;
    Gtk::Label m_releaseDateLabel;
    Gtk::Label m_consumedDateLabel;

    void initLayout();
    static std::unique_ptr<DetailsFormWidget> createFormFor(MediaCategory category);
};
