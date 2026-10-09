cmake_minimum_required(VERSION 4.2)
project(stapikmedia VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_NAME_DISPLAY "Stapik Media")
set(PROJECT_AUTHOR "Sebastian Smoliński")
set(PROJECT_GITHUB_URL "https://github.com/Stapik-Group/stapik-media")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

find_package(CURL REQUIRED)

include(FetchContent)
FetchContent_Declare(
        stapikcommon
        GIT_REPOSITORY https://github.com/Stapik-Group/stapik-common.git
        GIT_TAG v1.3.2
)
FetchContent_MakeAvailable(stapikcommon)

stapik_add_application(
        NAME stapikmedia
        DISPLAY_NAME "${PROJECT_NAME_DISPLAY}"
        DESCRIPTION "A retro-styled desktop media-tracking application"
        VERSION ${PROJECT_VERSION}
        APPLICATION_ID pl.stapik.media
        RESOURCES_DIRECTORY resources
        PACKAGING_DIRECTORY packaging
        MAINTAINER "${PROJECT_AUTHOR} <sebastian.smolinski6@gmail.com>"
        HOMEPAGE ${PROJECT_GITHUB_URL}
        SECTION utils
        DESKTOP_CATEGORIES Office
        LINK_LIBRARIES PkgConfig::GTKMM CURL::libcurl nlohmann_json::nlohmann_json
        SOURCES
                src/core/model/ReleaseDate.cpp
                src/core/model/MediaDocument.cpp
                src/core/model/MediaModel.cpp
                src/infrastructure/storage/MediaEntrySerializer.cpp
                src/infrastructure/storage/MediaDocumentSchema.cpp
                src/infrastructure/storage/MediaRepository.cpp
                src/ui/action/MenuActionHandler.cpp
                src/ui/dialog/details/AlbumDetailsForm.cpp
                src/ui/dialog/details/BookDetailsForm.cpp
                src/ui/dialog/details/GameDetailsForm.cpp
                src/ui/dialog/details/ScreenDetailsForm.cpp
                src/ui/dialog/MediaEntryDialog.cpp
                src/ui/util/DateFormatter.cpp
                src/ui/util/EntryFilterService.cpp
                src/ui/view/CategorySidebar.cpp
                src/ui/view/MediaListView.cpp
                src/ui/widget/ConsumedDatePicker.cpp
                src/ui/widget/MainMenu.cpp
                src/ui/widget/MediaEntryRow.cpp
                src/ui/widget/ReleaseDatePicker.cpp
                src/ui/widget/EntryFilterBar.cpp
                src/ui/widget/PaginationBar.cpp
                src/ui/window/MainWindow.cpp
                src/main.cpp
)
