cmake_minimum_required(VERSION 4.2)
project(stapikmedia VERSION 1.3.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_NAME_DISPLAY "Stapik Media")
set(PROJECT_AUTHOR "Sebastian Smoliński")
set(PROJECT_GITHUB_URL "https://github.com/Stapik-Group/stapik-media")
set(PROJECT_APPLICATION_ID "pl.stapik.media")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

find_package(CURL REQUIRED)

include(FetchContent)
FetchContent_Declare(
        stapikcommon
        GIT_REPOSITORY https://github.com/Stapik-Group/stapik-common.git
        GIT_TAG v1.6.0
)
FetchContent_MakeAvailable(stapikcommon)

add_library(stapikmediacore STATIC
        src/core/command/EntryCommands.cpp
        src/core/export/CsvReader.cpp
        src/core/export/EntryCsv.cpp
        src/core/export/EntryCsvImport.cpp
        src/core/model/EntryIdentity.cpp
        src/core/model/MediaDocument.cpp
        src/core/model/MediaModel.cpp
        src/core/model/ReleaseDate.cpp
        src/core/model/YearPartitioning.cpp
        src/infrastructure/storage/MediaDocumentSchema.cpp
        src/infrastructure/storage/MediaEntrySerializer.cpp
        src/infrastructure/storage/MediaRepository.cpp
        src/ui/util/ConsumptionText.cpp
        src/ui/util/EntryFilterService.cpp
        src/ui/util/EntrySuggestions.cpp
        src/ui/util/Pagination.cpp
)

target_include_directories(stapikmediacore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(stapikmediacore PUBLIC stapikcommon PkgConfig::GTKMM CURL::libcurl nlohmann_json::nlohmann_json)
stapik_enable_warnings(stapikmediacore)

stapik_add_application(
        NAME stapikmedia
        DISPLAY_NAME "${PROJECT_NAME_DISPLAY}"
        DESCRIPTION "A retro-styled desktop media-tracking application"
        LOCALIZED_DESCRIPTIONS
                "pl=Retro program do śledzenia obejrzanych filmów, przeczytanych książek i innych mediów"
                "de=Ein Retro-Programm zum Festhalten von Filmen, Büchern und anderen Medien"
        KEYWORDS media movies series books music albums games tracker
        LOCALIZED_KEYWORDS
                "pl=media,filmy,seriale,książki,muzyka,albumy,gry,dziennik"
                "de=Medien,Filme,Serien,Bücher,Musik,Alben,Spiele,Tagebuch"
        VERSION ${PROJECT_VERSION}
        APPLICATION_ID "${PROJECT_APPLICATION_ID}"
        RESOURCES_DIRECTORY resources
        PACKAGING_DIRECTORY packaging
        MAINTAINER "${PROJECT_AUTHOR} <sebastian.smolinski6@gmail.com>"
        HOMEPAGE ${PROJECT_GITHUB_URL}
        SECTION utils
        DESKTOP_CATEGORIES Office
        LINK_LIBRARIES stapikmediacore PkgConfig::GTKMM CURL::libcurl nlohmann_json::nlohmann_json
        SOURCES
                src/app/MediaAppInfo.cpp
                src/ui/action/ArchiveActionHandler.cpp
                src/ui/action/ExportActionHandler.cpp
                src/ui/action/ImportActionHandler.cpp
                src/ui/action/MenuActionHandler.cpp
                src/ui/dialog/details/AlbumDetailsForm.cpp
                src/ui/dialog/details/BookDetailsForm.cpp
                src/ui/dialog/details/GameDetailsForm.cpp
                src/ui/dialog/details/ScreenDetailsForm.cpp
                src/ui/dialog/ImportPreviewDialog.cpp
                src/ui/dialog/MediaEntryDialog.cpp
                src/ui/print/EntryPrintout.cpp
                src/ui/util/DateFormatter.cpp
                src/ui/util/EntryText.cpp
                src/ui/view/CategorySidebar.cpp
                src/ui/view/MediaListView.cpp
                src/ui/widget/MainMenu.cpp
                src/ui/widget/MediaEntryRow.cpp
                src/ui/widget/ReleaseDatePicker.cpp
                src/ui/widget/EntryFilterBar.cpp
                src/ui/widget/PaginationBar.cpp
                src/ui/widget/SuggestionCompletion.cpp
                src/ui/window/MainWindow.cpp
                src/main.cpp
)

target_compile_definitions(stapikmedia PRIVATE
        STAPIK_APP_APPLICATION_ID="${PROJECT_APPLICATION_ID}"
        STAPIK_APP_AUTHOR="${PROJECT_AUTHOR}"
        STAPIK_APP_REPOSITORY_URL="${PROJECT_GITHUB_URL}"
)

option(STAPIK_MEDIA_BUILD_TESTS "Build the Stapik Media unit tests" OFF)
if(STAPIK_MEDIA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
