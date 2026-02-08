# Test suite
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/test_types_typeinfo.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test_types_typeinfo.h
    COMMAND typeinfo_metaprogram
        -I${PROJECT_SOURCE_DIR}/include
        -I${TYPEINFO_CLANG_BUILTIN_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/test_types.h
        -o ${CMAKE_CURRENT_SOURCE_DIR}/test_types_typeinfo
    DEPENDS
        typeinfo_metaprogram
        ${CMAKE_CURRENT_SOURCE_DIR}/test_types.h
        ${PROJECT_SOURCE_DIR}/include/typeinfo.h
    COMMENT "Generating typeinfo for test_types"
)

add_executable(typeinfo_test EXCLUDE_FROM_ALL
    test.c
    test_types_typeinfo.c
)

target_compile_options(typeinfo_test PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wno-attributes -Wno-unused-function -Wno-pragmas>
)

target_link_libraries(typeinfo_test PRIVATE typeinfo)

# Custom test target - runs test binary directly with full output (no CTest!)
add_custom_target(test
    COMMAND typeinfo_test
    DEPENDS typeinfo_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running typeinfo tests..."
)
