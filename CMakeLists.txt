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
    src/Core/Component.cpp
    src/Core/ErrorCollector.cpp
    src/Core/Reference.cpp
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
    src/Support/ErrorReporter.cpp
    src/Support/LocalFileSystem.cpp
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
    src/Parsing/ASTVisitor.cpp
    src/Parsing/ComponentParser.cpp
    src/Parsing/PropertyParser.cpp
    src/Parsing/ReferenceParser.cpp
    src/Parsing/AnnotationValidator.cpp
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
    src/Serialization/JsonManifestBuilder.cpp
    src/Serialization/JsonManifestWriter.cpp
    src/Serialization/ManifestMerger.cpp
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