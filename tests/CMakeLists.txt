include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
        FIND_PACKAGE_ARGS NAMES GTest
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(stapikmedia_tests
        core/EntryCsvTest.cpp
        core/MediaCategoryKeyTest.cpp
        core/MediaDocumentTest.cpp
        core/MediaModelCloudTest.cpp
        core/MediaModelTest.cpp
        core/YearPartitioningTest.cpp
        infrastructure/MediaDocumentSchemaTest.cpp
        infrastructure/MediaEntrySerializerTest.cpp
        infrastructure/MediaRepositoryTest.cpp
        resources/MediaResourcesTest.cpp
        ui/EntryFilterServiceTest.cpp
        ui/PaginationTest.cpp
)

target_link_libraries(stapikmedia_tests PRIVATE
        stapikmediacore
        GTest::gtest_main
)

# The fake cloud storage, the fake clock and the other helpers are shared with the stapik-common tests.
target_include_directories(stapikmedia_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${stapikcommon_SOURCE_DIR}/tests
)

target_compile_definitions(stapikmedia_tests PRIVATE
        STAPIK_MEDIA_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        STAPIK_COMMON_RESOURCES_DIR="${STAPIK_COMMON_RESOURCES_DIR}"
)

stapik_enable_warnings(stapikmedia_tests)

gtest_discover_tests(stapikmedia_tests)
