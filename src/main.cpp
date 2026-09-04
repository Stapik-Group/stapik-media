#include <gtkmm.h>

#include "stapik/storage/AppPaths.hpp"
#include "stapik/theme/ThemeManager.hpp"
#include "stapik/ui/style/AppStyleProvider.hpp"

#include "infrastructure/network/CloudSchemaMigrationGuard.hpp"
#include "ui/window/MainWindow.hpp"

namespace
{
    constexpr auto APP_NAME = "stapikmedia";
}

int main(const int argc, char *argv[])
{
    CloudSchemaMigrationGuard::ensureCompatible();
    const auto app = Gtk::Application::create("pl.stapik.media");

    AppStyleProvider styleProvider(AppPaths::resourcesDir());

    app->signal_activate().connect([&]
    {
        styleProvider.apply(ThemeManager::instance(APP_NAME).getTheme());
        ThemeManager::instance().signalThemeChanged().connect([&styleProvider] { styleProvider.apply(ThemeManager::instance().getTheme()); });
        auto* window = new MainWindow();
        app->add_window(*window);
        window->show();
    });

    return app->run(argc, argv);
}