#pragma once

#include "details/DetailsFormWidget.hpp"

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <optional>
#include <memory>

#include "../widget/ConsumedDatePicker.hpp"
#include "../widget/ReleaseDatePicker.hpp"

class MediaEntryDialog : public Gtk::Dialog
{
public:
    MediaEntryDialog(Window& parent, MediaCategory category);
    MediaEntryDialog(Window& parent, const MediaEntry& existing);

    [[nodiscard]] std::optional<MediaEntry> getResult() const;

private:
    static constexpr int CONTENT_SPACING = 8;
    static constexpr int CONTENT_MARGIN = 16;
    static constexpr int DEFAULT_WIDTH = 420;

    MediaCategory m_category;
    Gtk::Box m_contentBox;
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
