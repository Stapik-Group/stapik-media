#include <gtkmm.h>

#include "stapik/app/AppContext.hpp"
#include "stapik/app/SingleInstanceGuard.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/theme/ThemeManager.hpp"
#include "stapik/ui/menu/StandardMenu.hpp"
#include "stapik/ui/style/AppStyleProvider.hpp"

#include "infrastructure/network/CloudSchemaMigrationGuard.hpp"
#include "ui/window/MainWindow.hpp"

namespace
{
    constexpr auto APPLICATION_ID = "pl.stapik.media";
    constexpr auto AUTHOR = "Sebastian Smoliński";
    constexpr auto REPOSITORY_URL = "https://github.com/Stapik-Group/stapik-media";
}

int main(const int argc, char *argv[])
{
    stapik::app::AppContext::initialize({
        .applicationId = APPLICATION_ID,
        .internalName = STAPIK_APP_NAME,
        .displayName = STAPIK_APP_DISPLAY_NAME,
        .version = STAPIK_APP_VERSION,
        .author = AUTHOR,
        .repositoryUrl = REPOSITORY_URL
    });

    const auto instanceGuard = stapik::app::SingleInstanceGuard::acquire(APPLICATION_ID, STAPIK_APP_DISPLAY_NAME);
    if (instanceGuard == nullptr)
        return 0;

    CloudSchemaMigrationGuard::ensureCompatible();
    const auto app = Gtk::Application::create(APPLICATION_ID);

    auto styleProvider = AppStyleProvider::withCommonThemes(AppPaths::resourcesDir());
    StandardMenu::installShortcuts(*app);

    app->signal_activate().connect([&]
    {
        styleProvider.apply(ThemeManager::instance().themeId());
        ThemeManager::instance().signalThemeChanged().connect([&styleProvider] { styleProvider.apply(ThemeManager::instance().themeId()); });
        auto* window = new MainWindow(styleProvider.themes());
        app->add_window(*window);
        window->show();
    });

    return app->run(argc, argv);
}
