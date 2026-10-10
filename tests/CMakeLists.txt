include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
        FIND_PACKAGE_ARGS NAMES GTest
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

set(MEDIA_SOURCES ${PROJECT_SOURCE_DIR}/src)

add_executable(stapikmedia_tests
        core/MediaCategoryKeyTest.cpp
        core/MediaDocumentTest.cpp
        core/YearPartitioningTest.cpp
        infrastructure/MediaDocumentSchemaTest.cpp
        infrastructure/MediaEntrySerializerTest.cpp
        ui/EntryFilterServiceTest.cpp
        ${MEDIA_SOURCES}/core/model/MediaDocument.cpp
        ${MEDIA_SOURCES}/core/model/ReleaseDate.cpp
        ${MEDIA_SOURCES}/core/model/YearPartitioning.cpp
        ${MEDIA_SOURCES}/infrastructure/storage/MediaDocumentSchema.cpp
        ${MEDIA_SOURCES}/infrastructure/storage/MediaEntrySerializer.cpp
        ${MEDIA_SOURCES}/ui/util/EntryFilterService.cpp
)

target_include_directories(stapikmedia_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${MEDIA_SOURCES})
target_link_libraries(stapikmedia_tests PRIVATE stapik::common GTest::gtest_main)
target_compile_options(stapikmedia_tests PRIVATE -Wall -Wextra -Wpedantic)

gtest_discover_tests(stapikmedia_tests)
