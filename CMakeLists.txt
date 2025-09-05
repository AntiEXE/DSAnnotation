cmake_minimum_required(VERSION 3.10)
project(AnnotationParser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "C:/gtest" ${CMAKE_PREFIX_PATH})
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(GTest CONFIG REQUIRED)

# Create a library for the core functionality (excluding main.cpp)
add_library(parser_lib
    ASTVisitor.cpp
    ComponentParser.cpp
    Reference.cpp
    Component.cpp
    JsonGenerator.cpp
    PropertyParser.cpp
    ReferenceParser.cpp
    ErrorCollector.cpp
    ErrorReporter.cpp
    SyntaxChecker.cpp
    ManifestMerger.cpp
)

# Create the main executable
add_executable(parser main.cpp)

target_include_directories(parser_lib
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_include_directories(parser
    PRIVATE 
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

# Platform-dependent linking (keeping your existing logic)
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
        # add more if you get linker errors
    )
else()
    set(CLANG_LIBS clang-cpp)
    set(LLVM_LIBS LLVM)
endif()

target_link_libraries(parser_lib
    PUBLIC
    ${CLANG_LIBS}
    ${LLVM_LIBS}
)

target_link_libraries(parser 
    PRIVATE 
    parser_lib
)

target_compile_definitions(parser_lib
    PRIVATE
    ${LLVM_DEFINITIONS}
)

# Enable testing
enable_testing()
add_subdirectory(tests)