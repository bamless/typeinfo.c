#ifndef PRINT_TYPES_H_
#define PRINT_TYPES_H_

#include <stdint.h>

#include "typeinfo.h"

typedef struct TI_ANN(BazAnnotation) {
    intptr_t iptr;
    void* ptr;
} Baz;

typedef struct {
    size_t x;
    size_t y;
    Baz baz[3];
    struct {
        struct {
            int x;
        } anon2;
    } anon;
} Bar;

typedef struct TI_ROOT {
    const char name[256] TI_ANN(CStr);
    int test;
    Bar* bar;
} Foo;

typedef enum TI_ROOT {
    COLOR_RED TI_ANN(Primary) TI_ANN(Secondary),
    COLOR_GREEN TI_ANN(Primary),
    COLOR_BLUE TI_ANN(Primary),
    COLOR_YELLOW,
    COLOR_CYAN,
    COLOR_MAGENTA,
} Color;

typedef union TI_ROOT {
    int i;
    float f;
    char c[4];
    struct {
        int x;
        int y;
    } anon_struct;
} TestUnion;

typedef struct TI_ROOT {
    enum { ANON_A, ANON_B, ANON_C } anon_enum_field;
    int other_field;
} TestAnonymousEnum;

typedef struct TI_ROOT {
    int before;
    struct {
        union {
            int x TI_ANN(X1);
            int y TI_ANN(Y1);
        };
    };
    int after;
} TestUnnamedAnonymous;

typedef struct TI_ROOT {
    const int ci;
    volatile int vi;
    const char* cstr_ptr;
    char* const const_ptr;
    const char* const both;
} TestQualifiers;

#endif  // PRINT_TYPES_H_
