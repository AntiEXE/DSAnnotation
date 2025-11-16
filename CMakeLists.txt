cmake_minimum_required(VERSION 3.16)
project(DSAnnotation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
set(CMAKE_PREFIX_PATH "C:/gtest" ${CMAKE_PREFIX_PATH})
find_package(GTest CONFIG REQUIRED)

set(DSANNOTATION_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(dsannotation_core
    src/core/Component.cpp
    src/core/ErrorCollector.cpp
    src/core/Reference.cpp
)
target_include_directories(dsannotation_core
    PUBLIC
        ${DSANNOTATION_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

add_library(dsannotation_support
    src/support/ErrorReporter.cpp
    src/support/LocalFileSystem.cpp
)
target_link_libraries(dsannotation_support
    PUBLIC
        dsannotation_core
)
target_include_directories(dsannotation_support
    PUBLIC
        ${DSANNOTATION_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
)

add_library(dsannotation_parsing
    src/parsing/ASTVisitor.cpp
    src/parsing/ComponentParser.cpp
    src/parsing/PropertyParser.cpp
    src/parsing/ReferenceParser.cpp
    src/parsing/AnnotationValidator.cpp
)
target_link_libraries(dsannotation_parsing
    PUBLIC
        dsannotation_core
        dsannotation_support
)
target_include_directories(dsannotation_parsing
    PUBLIC
        ${DSANNOTATION_INCLUDE_DIR}
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
)

add_library(dsannotation_serialization
    src/serialization/JsonManifestBuilder.cpp
    src/serialization/JsonManifestWriter.cpp
    src/serialization/ManifestMerger.cpp
)
target_link_libraries(dsannotation_serialization
    PUBLIC
        dsannotation_core
        dsannotation_support
)
target_include_directories(dsannotation_serialization
    PUBLIC
        ${DSANNOTATION_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
)

add_executable(dsannotation
    app/main.cpp
)
target_include_directories(dsannotation
    PRIVATE
        ${DSANNOTATION_INCLUDE_DIR}
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)
    set(CLANG_LIBS
        clangTooling
        clangFrontend
        clangDriver
        clangParse
        clangSerialization
        clangSema
        clangEdit
        clangAnalysis
        clangAST
        clangLex
        clangBasic
    )
    llvm_map_components_to_libnames(LLVM_LIBS
        support
        core
        option
        binaryformat
        mc
        bitreader
        profiledata
        demangle
    )
else()
    set(CLANG_LIBS clang-cpp)
    set(LLVM_LIBS LLVM)
endif()

target_link_libraries(dsannotation
    PRIVATE
        dsannotation_parsing
        dsannotation_serialization
        ${CLANG_LIBS}
        ${LLVM_LIBS}
)

target_compile_definitions(dsannotation
    PRIVATE
        ${LLVM_DEFINITIONS}
)

enable_testing()
add_subdirectory(tests)


# Apply coverage flags to test executable
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(dsannotation_tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(dsannotation_tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
endif()
