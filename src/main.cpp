#include "app/MediaAppInfo.hpp"
#include "ui/window/MainWindow.hpp"

#include "stapik/app/StapikApplication.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/theme/ThemeRegistry.hpp"

#include <curl/curl.h>

#include <glibmm/main.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    constexpr std::string_view SELF_TEST_FLAG = "--self-test";
    constexpr std::string_view SELF_TEST_PROBE_KEY = "window.button.add";
    constexpr unsigned int SELF_TEST_DURATION_MS = 2000;
    constexpr std::size_t SELF_TEST_MIN_THEMES = 5;

    bool extractSelfTestFlag(std::vector<char*>& arguments)
    {
        return std::erase_if(arguments, [](const char* argument) { return std::string_view(argument) == SELF_TEST_FLAG; }) > 0;
    }

    bool verifyInstallation(const stapik::theme::ThemeRegistry& themes)
    {
        auto allPassed = true;
        const auto check = [&allPassed](const bool passed, const std::string_view description)
        {
            if (passed)
            {
                stapik::log::info("[self-test] ok: {}", description);
                return;
            }

            stapik::log::error("[self-test] FAILED: {}", description);
            allPassed = false;
        };

        std::error_code errorCode;
        check(std::filesystem::is_directory(AppPaths::resourcesDir() / "locales", errorCode), "application locales");
        check(std::filesystem::is_directory(AppPaths::resourcesDir() / "tutorial" / "en", errorCode), "application tutorial");
        check(std::filesystem::is_directory(AppPaths::commonResourcesDir() / "themes", errorCode), "common themes");
        check(themes.themes().size() >= SELF_TEST_MIN_THEMES, "themes registered");

        const auto probe = LocaleManager::instance().translate(std::string(SELF_TEST_PROBE_KEY));
        check(!probe.empty() && probe != SELF_TEST_PROBE_KEY, "translations");

        const auto* curlInfo = curl_version_info(CURLVERSION_NOW);
        check(curlInfo != nullptr && curlInfo->ssl_version != nullptr, "TLS support in libcurl");

        return allPassed;
    }
}

int main(const int argc, char *argv[])
{
    std::vector<char*> arguments(argv, argv + argc);
    const auto selfTest = extractSelfTestFlag(arguments);
    arguments.push_back(nullptr);

    const auto app = stapik::app::StapikApplication::create(makeMediaAppInfo(), { .checkForUpdates = !selfTest });

    auto exitCode = 0;

    app->signal_activate().connect([&]
    {
        if (auto* existingWindow = app->get_active_window())
        {
            existingWindow->present();
            return;
        }

        auto* window = new MainWindow(app->themes());
        app->add_window(*window);
        window->show();

        if (!selfTest)
            return;

        if (!verifyInstallation(app->themes()))
            exitCode = 1;

        Glib::signal_timeout().connect_once([&app] { app->quit(); }, SELF_TEST_DURATION_MS);
    });

    const auto result = app->run(static_cast<int>(arguments.size()) - 1, arguments.data());
    return result != 0 ? result : exitCode;
}
