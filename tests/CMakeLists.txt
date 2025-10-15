find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(dsannotation_tests
    PropertyParserTest.cpp
)

# Modern CMake targets (if available)
if(TARGET GTest::gtest AND TARGET GTest::gtest_main)
    target_link_libraries(dsannotation_tests
        PRIVATE
        dsannotation_core
        dsannotation_support
        dsannotation_parsing
        dsannotation_serialization
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
    )
else()
    # Fallback for older GTest installations
    target_include_directories(dsannotation_tests PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(dsannotation_tests
        PRIVATE
        dsannotation_core
        dsannotation_support
        dsannotation_parsing
        dsannotation_serialization
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        Threads::Threads
    )
endif()

add_test(NAME dsannotation_tests COMMAND dsannotation_tests)
set_tests_properties(dsannotation_tests PROPERTIES ENVIRONMENT "GTEST_COLOR=1")

# Include GoogleTest module for test discovery
include(GoogleTest)
gtest_discover_tests(dsannotation_tests)