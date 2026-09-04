cmake_minimum_required(VERSION 3.25)
project(stapikmedia VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_NAME_DISPLAY "Stapik Media")
set(PROJECT_AUTHOR "Sebastian Smoliński")
set(PROJECT_GITHUB_URL "https://github.com/Stapik-Group/stapik-media")

include(FetchContent)
FetchContent_Declare(
        stapikcommon
        GIT_REPOSITORY https://github.com/Stapik-Group/stapik-common.git
        GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(stapikcommon)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

add_executable(stapikmedia
        src/core/model/ReleaseDate.cpp
        src/core/model/MediaModel.cpp
        src/infrastructure/storage/MediaEntrySerializer.cpp
        src/infrastructure/storage/MediaSyncCoordinator.cpp
        src/infrastructure/storage/Storage.cpp
        src/ui/action/MenuActionHandler.cpp
        src/ui/dialog/details/AlbumDetailsForm.cpp
        src/ui/dialog/details/BookDetailsForm.cpp
        src/ui/dialog/details/GameDetailsForm.cpp
        src/ui/dialog/details/ScreenDetailsForm.cpp
        src/ui/dialog/AboutDialog.cpp
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

target_link_libraries(stapikmedia PRIVATE
        PkgConfig::GTKMM
        stapikcommon
)

configure_file(
        src/ui/dialog/Version.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.hpp
)
target_include_directories(stapikmedia PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

add_custom_command(TARGET stapikmedia POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:stapikmedia>/resources
)

install(TARGETS stapikmedia
        RUNTIME DESTINATION lib/stapikmedia
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/
        DESTINATION lib/stapikmedia/resources
)

install(PROGRAMS ${CMAKE_SOURCE_DIR}/packaging/stapikmedia-launcher.sh
        DESTINATION bin
        RENAME stapikmedia
)

install(FILES ${CMAKE_SOURCE_DIR}/packaging/stapikmedia.desktop
        DESTINATION share/applications
)

install(FILES ${CMAKE_SOURCE_DIR}/packaging/icon.png
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME stapikmedia.png
)

set(CPACK_PACKAGE_NAME "stapikmedia")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR ${PROJECT_AUTHOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A retro-styled desktop media-tracking application")
set(CPACK_PACKAGE_CONTACT "sebastian.smolinski6@gmail.com")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${PROJECT_AUTHOR} <sebastian.smolinski6@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

include(CPack)