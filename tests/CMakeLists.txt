find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(unit_tests
    PropertyParserTest.cpp
    # ComponentParserTest.cpp
    # Add other test files here
)

target_include_directories(unit_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}
)

# Modern CMake targets (if available)
if(TARGET GTest::gtest AND TARGET GTest::gtest_main)
    target_link_libraries(unit_tests
        PRIVATE
        parser_lib
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
    )
else()
    # Fallback for older GTest installations
    target_include_directories(unit_tests PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(unit_tests
        PRIVATE
        parser_lib
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        Threads::Threads
    )
endif()

add_test(NAME unit_tests COMMAND unit_tests)
set_tests_properties(unit_tests PROPERTIES ENVIRONMENT "GTEST_COLOR=1")

# Include GoogleTest module for test discovery
include(GoogleTest)
gtest_discover_tests(unit_tests)