#include "MediaEntryDialog.hpp"

#include "details/ScreenDetailsForm.hpp"
#include "details/BookDetailsForm.hpp"
#include "details/AlbumDetailsForm.hpp"
#include "details/GameDetailsForm.hpp"

#include "stapik/locale/LocaleManager.hpp"

MediaEntryDialog::MediaEntryDialog(Window &parent, const MediaCategory category) :
    StapikDialog(parent, LocaleManager::instance().translate("dialog.entry.title.add")),
    m_category(category),
    m_detailsContainer(Gtk::Orientation::VERTICAL, DETAILS_SPACING)
{
    initLayout();
}

MediaEntryDialog::MediaEntryDialog(Window &parent, const MediaEntry &existing) :
    StapikDialog(parent, LocaleManager::instance().translate("dialog.entry.title.edit")),
    m_category(existing.category),
    m_id(existing.id),
    m_detailsContainer(Gtk::Orientation::VERTICAL, DETAILS_SPACING)
{
    initLayout();

    m_titleEntry.set_text(existing.title);
    m_releaseDatePicker.loadFrom(existing.releaseDate);
    m_consumedMonthPicker.setMonth(existing.consumed);
    m_detailsForm->loadFrom(existing.details);
}

void MediaEntryDialog::initLayout()
{
    const auto &loc = LocaleManager::instance();

    m_titleLabel.set_text(loc.translate("dialog.entry.title.label"));
    m_titleLabel.set_halign(Gtk::Align::START);

    m_releaseDateLabel.set_text(loc.translate("dialog.entry.releaseDate.label"));
    m_releaseDateLabel.set_halign(Gtk::Align::START);
    m_releaseDateLabel.add_css_class("panel-title");

    m_consumedDateLabel.set_text(loc.translate("dialog.entry.consumedDate.label"));
    m_consumedDateLabel.set_halign(Gtk::Align::START);
    m_consumedDateLabel.add_css_class("panel-title");

    contentBox().add_css_class("media-entry-dialog-content");
    contentBox().append(m_titleLabel);
    contentBox().append(m_titleEntry);
    contentBox().append(m_releaseDateLabel);
    contentBox().append(m_releaseDatePicker);
    contentBox().append(m_consumedDateLabel);
    contentBox().append(m_consumedMonthPicker);

    m_detailsForm = createFormFor(m_category);
    m_detailsForm->add_css_class("details-form");
    m_detailsContainer.append(*m_detailsForm);
    contentBox().append(m_detailsContainer);

    addCancelButton();
    addOkButton(loc.translate("dialog.entry.button.save"));

    m_titleEntry.set_activates_default(true);
    set_default_size(DEFAULT_WIDTH, -1);
}

std::unique_ptr<DetailsFormWidget> MediaEntryDialog::createFormFor(const MediaCategory category)
{
    switch (category)
    {
        using enum MediaCategory;
        case Movie:
        case Series:
        case Cartoon:
            return std::make_unique<ScreenDetailsForm>();
        case Book:
            return std::make_unique<BookDetailsForm>();
        case Album:
            return std::make_unique<AlbumDetailsForm>();
        case Game:
            return std::make_unique<GameDetailsForm>();
    }
    throw std::invalid_argument("Unknown MediaCategory in createFormFor");
}

std::optional<MediaEntry> MediaEntryDialog::getResult() const
{
    const auto title = m_titleEntry.get_text();
    if (title.empty())
        return std::nullopt;

    return MediaEntry{
        m_category,
        std::string(title),
        m_releaseDatePicker.toReleaseDate(),
        m_consumedMonthPicker.month(),
        m_detailsForm->toDetails(),
        m_id
    };
}
