# Example: print_types
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/print_types_typeinfo.c
        ${CMAKE_CURRENT_SOURCE_DIR}/print_types_typeinfo.h
    COMMAND typeinfo_metaprogram
        -I${PROJECT_SOURCE_DIR}/include
        -I${TYPEINFO_CLANG_BUILTIN_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/print_types.h
        -o ${CMAKE_CURRENT_SOURCE_DIR}/print_types_typeinfo
    DEPENDS
        typeinfo_metaprogram
        ${CMAKE_CURRENT_SOURCE_DIR}/print_types.h
        ${PROJECT_SOURCE_DIR}/include/typeinfo.h
    COMMENT "Generating typeinfo for print_types"
)
add_executable(print_types
    print_types.c
    print_types_typeinfo.c
)
target_compile_options(print_types PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wno-unused-variable>
)
target_link_libraries(print_types PRIVATE typeinfo)
