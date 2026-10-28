#pragma once

#include "details/DetailsFormWidget.hpp"

#include "stapik/domain/YearMonth.hpp"
#include "stapik/ui/dialog/StapikDialog.hpp"
#include "stapik/ui/widget/MonthPicker.hpp"

#include <gtkmm/box.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <optional>
#include <memory>
#include <vector>

#include "../widget/ReleaseDatePicker.hpp"

class MediaEntryDialog : public StapikDialog
{
public:
    MediaEntryDialog(Window& parent, MediaCategory category);
    MediaEntryDialog(Window& parent, const MediaEntry& existing);

    [[nodiscard]] std::optional<MediaEntry> getResult() const;
    void setSuggestions(const std::vector<MediaEntry>& entries);

private:
    static constexpr int DEFAULT_WIDTH = 420;
    static constexpr int DETAILS_SPACING = 8;
    static constexpr int NOTE_HEIGHT = 80;
    static constexpr guint NO_RATING_INDEX = 0;

    MediaCategory m_category;
    std::string m_id;
    Gtk::Label m_titleLabel;
    Gtk::Entry m_titleEntry;
    ReleaseDatePicker m_releaseDatePicker;
    MonthPicker m_consumedMonthPicker{ stapik::domain::YearMonth::current() };
    Gtk::Box m_detailsContainer;
    std::unique_ptr<DetailsFormWidget> m_detailsForm;
    Gtk::Label m_releaseDateLabel;
    Gtk::Label m_consumedDateLabel;
    Gtk::Label m_ratingLabel;
    Gtk::DropDown m_ratingDropDown;
    Gtk::Label m_noteLabel;
    Gtk::ScrolledWindow m_noteScroller;
    Gtk::TextView m_noteView;

    void initLayout();
    static std::unique_ptr<DetailsFormWidget> createFormFor(MediaCategory category);
};
