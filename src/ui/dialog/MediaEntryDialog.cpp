#include "MediaEntryDialog.hpp"

#include "details/ScreenDetailsForm.hpp"
#include "details/BookDetailsForm.hpp"
#include "details/AlbumDetailsForm.hpp"
#include "details/GameDetailsForm.hpp"

#include "stapik/locale/LocaleManager.hpp"

MediaEntryDialog::MediaEntryDialog(Window &parent, const MediaCategory category) :
    Dialog(LocaleManager::instance().translate("dialog.entry.title.add"), parent, true),
    m_category(category),
    m_contentBox(Gtk::Orientation::VERTICAL, CONTENT_SPACING),
    m_detailsContainer(Gtk::Orientation::VERTICAL, CONTENT_SPACING)
{
    initLayout();

    const auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    const std::chrono::year_month_day ymd{today};

    m_consumedDatePicker.loadFrom(ConsumedDate{
        static_cast<unsigned>(ymd.month()),
        static_cast<unsigned>(static_cast<int>(ymd.year()))
    });
}

MediaEntryDialog::MediaEntryDialog(Window &parent, const MediaEntry &existing) : Dialog(
        LocaleManager::instance().translate("dialog.entry.title.edit"), parent, true),
    m_category(existing.category),
    m_contentBox(Gtk::Orientation::VERTICAL, CONTENT_SPACING),
    m_detailsContainer(Gtk::Orientation::VERTICAL, CONTENT_SPACING)
{
    initLayout();

    m_titleEntry.set_text(existing.title);
    m_releaseDatePicker.loadFrom(existing.releaseDate);
    m_consumedDatePicker.loadFrom(existing.consumed);
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

    m_contentBox.set_margin(CONTENT_MARGIN);
    m_contentBox.add_css_class("media-entry-dialog-content");
    m_contentBox.append(m_titleLabel);
    m_contentBox.append(m_titleEntry);
    m_contentBox.append(m_releaseDateLabel);
    m_contentBox.append(m_releaseDatePicker);
    m_contentBox.append(m_consumedDateLabel);
    m_contentBox.append(m_consumedDatePicker);

    m_detailsForm = createFormFor(m_category);
    m_detailsForm->add_css_class("details-form");
    m_detailsContainer.append(*m_detailsForm);
    m_contentBox.append(m_detailsContainer);

    get_content_area()->append(m_contentBox);

    add_button(loc.translate("button.cancel"), Gtk::ResponseType::CANCEL);
    add_button(loc.translate("dialog.entry.button.save"), Gtk::ResponseType::OK);

    set_default_response(Gtk::ResponseType::OK);
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
        m_consumedDatePicker.toConsumedDate(),
        m_detailsForm->toDetails()
    };
}
