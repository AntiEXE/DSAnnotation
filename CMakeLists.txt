cmake_minimum_required(VERSION 3.10)
project(AnnotationParser)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
 
add_executable(parser
    main.cpp
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
 
target_include_directories(parser
    PRIVATE 
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)
 
# --- Platform-dependent linking ---
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
        demangle
        # add more if you get linker errors
    )
else()
    set(CLANG_LIBS clang-cpp)
    set(LLVM_LIBS LLVM)
endif()
 
target_link_libraries(parser
    PRIVATE
    ${CLANG_LIBS}
    ${LLVM_LIBS}
)
 
target_compile_definitions(parser
    PRIVATE
    ${LLVM_DEFINITIONS}
)