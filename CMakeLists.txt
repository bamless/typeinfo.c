cmake_minimum_required(VERSION 3.16)
project(typeinfo LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TYPEINFO_BUILD_EXAMPLES "Build typeinfo examples" ON)
option(TYPEINFO_BUILD_TESTS "Build typeinfo tests" OFF)

add_library(typeinfo INTERFACE)
target_include_directories(typeinfo INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(typeinfo INTERFACE c_std_99)

find_library(LIBCLANG_LIBRARY NAMES clang libclang REQUIRED)
find_path(LIBCLANG_INCLUDE_DIR NAMES clang-c/Index.h REQUIRED)

execute_process(
    COMMAND clang -print-resource-dir
    OUTPUT_VARIABLE CLANG_RESOURCE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _clang_result
)
if(NOT _clang_result EQUAL 0)
    message(FATAL_ERROR "Could not determine clang resource directory. Is clang installed?")
endif()
set(TYPEINFO_CLANG_BUILTIN_INCLUDE_DIR "${CLANG_RESOURCE_DIR}/include" CACHE PATH "Path to clang builtin headers")

add_executable(typeinfo_metaprogram typeinfo_metaprogram.c)
target_compile_options(typeinfo_metaprogram PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra>
)
target_include_directories(typeinfo_metaprogram PRIVATE ${LIBCLANG_INCLUDE_DIR})
target_link_libraries(typeinfo_metaprogram PRIVATE ${LIBCLANG_LIBRARY})

if(TYPEINFO_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(TYPEINFO_BUILD_TESTS)
    add_subdirectory(test)
endif()
