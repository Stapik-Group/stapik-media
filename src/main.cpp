#include <gtkmm.h>

#include "stapik/storage/AppPaths.hpp"
#include "stapik/ui/style/AppStyleProvider.hpp"

#include "infrastructure/network/CloudSchemaMigrationGuard.hpp"
#include "ui/window/MainWindow.hpp"

int main(const int argc, char *argv[])
{
    CloudSchemaMigrationGuard::ensureCompatible();
    const auto app = Gtk::Application::create("pl.stapik.media");

    app->signal_activate().connect([&]
    {
        AppStyleProvider::load((AppPaths::resourcesDir() / "style.css").string());

        auto* window = new MainWindow();
        app->add_window(*window);
        window->show();
    });

    return app->run(argc, argv);
}