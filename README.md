# typeinfo.c

[![Build](https://github.com/bamless/typeinfo.c/actions/workflows/build.yml/badge.svg)](https://github.com/bamless/typeinfo.c/actions/workflows/build.yml)

Compile-time type reflection for C. A metaprogram parses your C headers using
libclang, then generates static type information tables that you can query at
runtime -- struct members, enum values, union fields, annotations, sizes,
offsets, and more.

## Table of Contents

- [Overview](#overview)
- [Usage](#usage)
  - [Step 1: Annotate your headers](#step-1-annotate-your-headers)
  - [Step 2: Run the metaprogram](#step-2-run-the-metaprogram)
  - [Step 3: Use the generated type info](#step-3-use-the-generated-type-info)
- [Metaprogram Reference](#metaprogram-reference)
- [Type Info Data Model](#type-info-data-model)
- [Platform Setup](#platform-setup)
  - [Linux](#linux-ubuntudebian)
  - [macOS](#macos)
  - [Windows](#windows)
- [Building](#building)
  - [Prerequisites](#prerequisites)
  - [Copy & Paste](#copy-paste)
  - [CMake](#cmake)
  - [Make](#make)
- [Integrating into Your CMake Project](#integrating-into-your-cmake-project)

## Overview

typeinfo works in two phases:

1. **Code generation (build time):** A metaprogram (`typeinfo_metaprogram`)
   uses libclang to parse your C headers. It finds types annotated with
   `TI_ROOT`, recursively walks their members, and emits a pair of `.c`/`.h`
   files containing static type information tables.

2. **Runtime introspection:** Your application includes the generated files and
   the `typeinfo.h` header. You can then inspect any reflected type at runtime
   -- iterate struct members, look up enum values by name or number, check
   sizes and offsets, read custom annotations, and so on.

Only types explicitly marked with `TI_ROOT` (and their transitive dependencies)
are included in the generated output.

## Usage

### Step 1: Annotate your headers

Include `typeinfo.h` and use the `TI_ROOT` macro to mark types for reflection.
Use `TI_ANN(x)` to attach custom string annotations to types, members, or
enum values.

```c
#include "typeinfo.h"

typedef struct TI_ROOT {
    const char name[256] TI_ANN(CStr);
    int health;
    float speed;
} Player;

typedef enum TI_ROOT {
    WEAPON_SWORD TI_ANN(Melee),
    WEAPON_BOW   TI_ANN(Ranged),
    WEAPON_STAFF TI_ANN(Magic),
} WeaponType;
```

`TI_ROOT` marks a type as a root for code generation. The metaprogram will
process it and all types it transitively depends on (e.g., if `Player` had a
pointer to another struct, that struct would also be included).

`TI_ANN(x)` attaches a string annotation. You can use multiple annotations on
the same declaration. Annotations are available at runtime as NULL-terminated
string arrays. During normal compilation (without the metaprogram), both macros
expand to nothing.

### Step 2: Run the metaprogram

```bash
./typeinfo_metaprogram game_types.h -o game_types_typeinfo
```

This parses `game_types.h` and generates two files:

- `game_types_typeinfo.h` -- extern declarations for all reflected types
- `game_types_typeinfo.c` -- static data tables with type information

Each reflected type gets a global variable named `typeinfo_<TypeName>`. For
the example above, you would get `typeinfo_Player` and `typeinfo_WeaponType`.

> NOTE: If you get errors about standard headers not being found, you'll need to
> `-I/usr/lib/clang/<your-clang-version>` or, equivalently, the folder in which your compiler keeps
> those headers. This has to be done since some c standard headers are compiler-specific.
> The CMake build for example handles this automatically via `clang -print-resource-dir`.

### Step 3: Use the generated type info

Include the generated header and use the type info structures:

```c
#include "game_types.h"
#include "game_types_typeinfo.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    // Access struct metadata
    Type_Info_Struct* player_info = &typeinfo_Player;
    printf("Type: %s (size: %zu bytes)\n", player_info->name, player_info->base.size);

    for (size_t i = 0; i < player_info->members_count; i++) {
        Type_Info_Member* m = &player_info->members[i];
        printf("  .%s at offset %zu\n", m->name, m->offset);

        // Check annotations
        for (char** ann = m->annotations; *ann; ann++) {
            printf("    annotation: %s\n", *ann);
        }
    }

    // Access enum metadata
    Type_Info_Enum* weapon_info = &typeinfo_WeaponType;
    for (size_t i = 0; i < weapon_info->values_count; i++) {
        Type_Info_Enum_Value* v = &weapon_info->values[i];
        printf("  %s = %lld\n", v->name, v->value);
    }

    // Use type info for generic value printing, serialization, etc.
    Player p = { "Alice", 100, 5.5f };
    // ... walk members and print/serialize based on type tags
}
```

The generated type info references are all `Type_Info*` at the base level. Cast
to the appropriate variant based on the `tag` field:

```c
Type_Info* ti = (Type_Info*)&typeinfo_Player;

switch (ti->tag) {
case TYPE_TAG_STRUCT: {
    Type_Info_Struct* s = (Type_Info_Struct*)ti;
    // iterate s->members ...
    break;
}
case TYPE_TAG_ENUM: {
    Type_Info_Enum* e = (Type_Info_Enum*)ti;
    // iterate e->values ...
    break;
}
// ... handle other tags
}
```

See `examples/print_types.c` for a complete example that recursively prints
any value given its type info, including support for nested structs, arrays,
pointers, unions, and annotation-based special handling (e.g., printing
`char[]` fields as C strings when annotated with `CStr`).

## Metaprogram Reference

```
USAGE: typeinfo_metaprogram [OPTIONS] [FILE...]

OPTIONS:
  -o <out_name>        Base name for generated files (required).
                       Produces <out_name>.h and <out_name>.c
  -I<dir>              Add an include search path (forwarded to clang)
  -std=<std>           Set the C language standard (forwarded to clang)
  -no-builtin-types    Do not emit declarations/definitions for builtin
                       C types (void, int, float, char, etc.)
  -R                   Recursively walk directories for input files
  -h                   Print usage and exit
```

> NOTE: Depending on your system, the path to clang's builtin headers may need to be passed via `-I`.
> The CMake build handles this automatically via `clang -print-resource-dir`.

## Type Info Data Model

All type info structures share a common base:

```c
typedef struct {
    Type_Info_Tag tag;   // Discriminant (TYPE_TAG_STRUCT, TYPE_TAG_ENUM, etc.)
    size_t size;         // sizeof() the type
    size_t alignment;    // alignof() the type
} Type_Info;
```

The tag determines which variant to cast to:

| Tag                | Variant Type        | Extra Fields                                        |
|--------------------|---------------------|-----------------------------------------------------|
| `TYPE_TAG_VOID`    | `Type_Info_Void`    | (none)                                              |
| `TYPE_TAG_INTEGER` | `Type_Info_Integer` | `bool is_signed`                                    |
| `TYPE_TAG_FLOAT`   | `Type_Info_Float`   | (none)                                              |
| `TYPE_TAG_POINTER` | `Type_Info_Pointer` | `Type_Info* pointer_to`, `uint32_t qualifier_flags` |
| `TYPE_TAG_ARRAY`   | `Type_Info_Array`   | `size_t num_elements`, `Type_Info* element_type`    |
| `TYPE_TAG_STRUCT`  | `Type_Info_Struct`  | `name`, `annotations`, `members`, `members_count`   |
| `TYPE_TAG_UNION`   | `Type_Info_Union`   | `name`, `annotations`, `members`, `members_count`   |
| `TYPE_TAG_ENUM`    | `Type_Info_Enum`    | `name`, `annotations`, `values`, `values_count`     |

Struct and union members are described by `Type_Info_Member`:

```c
typedef struct {
    char** annotations;        // NULL-terminated array of annotation strings
    const char* name;          // Member name
    size_t offset;             // Byte offset within the struct/union
    Type_Info* type;           // Type info for this member
    uint32_t qualifier_flags;  // Qualifiers applied to this struct or union member; Bitmask of
                               // `Type_Info_Qualifier`
} Type_Info_Member;
```

Enum values are described by `Type_Info_Enum_Value`:

```c
typedef struct {
    char** annotations;   // NULL-terminated array of annotation strings
    const char* name;     // Enumerator name
    long long value;      // Numeric value
} Type_Info_Enum_Value;
```

The metaprogram also generates globals for all C builtin types (unless
`-no-builtin-types` is passed):

`typeinfo_void`, `typeinfo_bool`, `typeinfo_char`, `typeinfo_signed_char`,
`typeinfo_unsigned_char`, `typeinfo_short`, `typeinfo_unsigned_short`,
`typeinfo_int`, `typeinfo_unsigned_int`, `typeinfo_long`,
`typeinfo_unsigned_long`, `typeinfo_long_long`, `typeinfo_unsigned_long_long`,
`typeinfo_float`, `typeinfo_double`, `typeinfo_long_double`

`-no-builtin-types` is useful to avoid variable redefinition errors if you generate multiple
typeinfo files for a single project.

## Platform Setup

### Linux

Install the libclang development package using your distro package manager. Typical names are:
 - clang-devel  (Fedora)
 - libclang-dev (Ubuntu/Debian)

Since every distro seems to do the fuck it wants with clang libraries and headers, make sure you
have `libclang.so` and the `include/clang-c/` folder somewhere on your system after installation.

On Fedora, simply installing clang-devel does the trick.

On Ubuntu, and maybe other distros, the clang package may install a versioned LLVM (e.g.,
LLVM 18 on Ubuntu 24.04). The corresponding `llvm-config` binary will also be
versioned. Set `CMAKE_PREFIX_PATH` using the versioned `llvm-config`:

```bash
# For LLVM 18 (Ubuntu 24.04)
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$(llvm-config-18 --prefix)

# For LLVM 16 (Ubuntu 22.04)
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$(llvm-config-16 --prefix)
```

If you install a specific version of libclang (e.g., `libclang-21-dev`), use
the matching `llvm-config` version:

Common version-specific package names: for Ubuntu:

| Ubuntu Version | Default Package   | llvm-config Command  |
|----------------|-------------------|----------------------|
| 22.04 (Jammy)  | `libclang-14-dev` | `llvm-config-14`     |
| 24.04 (Noble)  | `libclang-18-dev` | `llvm-config-18`     |
| Latest / PPA   | `libclang-21-dev` | `llvm-config-21`     |

For other distros, good luck. lol.

### macOS

Install LLVM via Homebrew:

```bash
brew install llvm
```

Homebrew's LLVM is keg-only (not linked into `/usr/local`), so you need to
tell CMake where to find it:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$(brew --prefix llvm)
```

You may also need LLVM's `bin` directory on your PATH so that `clang` resolves
to Homebrew's version (used to locate builtin headers):

```bash
export PATH="$(brew --prefix llvm)/bin:$PATH"
```

### Windows

Install LLVM via Chocolatey:

```bash
choco install llvm -y
```

Then point CMake at the installation:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH="C:\Program Files\LLVM"
cmake --build build --config Release
```

Alternatively, download the LLVM installer from the
[LLVM releases page](https://github.com/llvm/llvm-project/releases) and set
`CMAKE_PREFIX_PATH` to wherever you installed it.

## Building

### Prerequisites

- C99 or later compiler (tested with GCC, Clang, and MSVC)
- libclang development libraries and headers

The following are needed only if you use the provided CMake or Make build:
- CMake 3.16+ (for the CMake build) or GNU Make
- The `clang` command available in your PATH (used to locate builtin headers)

### Copy Paste

Copy `typeinfo_metaprogram.c`, `extlib.h` and `include/typeinfo.h` into your project.
You will need to setup the compilation and execution of the metaprogram before the compilation of
your project.

> NOTE: `typeinfo.h` is *not* #included by the metaprogram, so you don't have to provide the path
> to it with `-I`.

### CMake

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

This builds the metaprogram and, by default, the `print_types` example. To
disable the example:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DTYPEINFO_BUILD_EXAMPLES=OFF
```

Run the example:

```bash
# Linux / macOS
./build/examples/print_types

# Windows
build\examples\Release\print_types.exe
```

If CMake cannot find libclang, you need to set `CMAKE_PREFIX_PATH` to the
LLVM installation prefix. See [Platform Setup](#platform-setup) for details.

### Make

The Makefile build uses GCC by default and directly links against `-lclang` (no pkg-config is
available for libclang, at least on my system... smh):

```bash
make typeinfo_metaprogram    # Build just the metaprogram
make examples/print_types    # Build the example (also runs code generation)
make clean                   # Remove build artifacts
```

## Integrating into Your CMake Project

To use typeinfo as part of your own CMake project, you can add it as a
subdirectory and set up a custom command to run the metaprogram during the
build:

```cmake
add_subdirectory(path/to/typeinfo)

# Run the metaprogram to generate type info from your header
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/my_types_typeinfo.c
        ${CMAKE_CURRENT_SOURCE_DIR}/my_types_typeinfo.h
    COMMAND typeinfo_metaprogram
        -I${CMAKE_CURRENT_SOURCE_DIR}/path/to/typeinfo/include
        -I${TYPEINFO_CLANG_BUILTIN_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/my_types.h
        -o ${CMAKE_CURRENT_SOURCE_DIR}/my_types_typeinfo
    DEPENDS
        typeinfo_metaprogram
        ${CMAKE_CURRENT_SOURCE_DIR}/my_types.h
    COMMENT "Generating typeinfo for my_types"
)

add_executable(my_app
    main.c
    my_types_typeinfo.c
)
target_link_libraries(my_app PRIVATE typeinfo)
```

The `typeinfo` interface library provides the include path for `typeinfo.h` and
sets the C99 standard requirement. The `TYPEINFO_CLANG_BUILTIN_INCLUDE_DIR`
cache variable is set by the typeinfo CMakeLists.txt and points to clang's
builtin headers directory.
