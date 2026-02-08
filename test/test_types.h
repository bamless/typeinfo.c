#ifndef TEST_TYPES_H_
#define TEST_TYPES_H_

#include <stdint.h>

#include "typeinfo.h"

typedef struct TI_ROOT {
    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
} TestIntegers;

typedef struct TI_ROOT {
    float f;
    double d;
} TestFloats;

typedef struct TI_ROOT {
    int* ptr;
    const int* const_ptr;
    volatile int* volatile_ptr;
    const volatile int* const_volatile_ptr;
    int* const ptr_const;
} TestPointers;

typedef struct TI_ROOT {
    int arr[10];
    char str[256] TI_ANN(CStr);
    float matrix[3][4];
} TestArrays;

typedef struct TI_ANN(StructAnnotation) {
    int x TI_ANN(XCoord);
    int y TI_ANN(YCoord);
} Point;

typedef struct TI_ROOT {
    Point point;
    Point* point_ptr;
} TestStructs;

typedef union TI_ROOT TI_ANN(UnionAnnotation) {
    int i;
    float f;
    char bytes[4];
} TestUnion;

typedef enum TI_ROOT {
    STATUS_OK TI_ANN(Success),
    STATUS_ERROR TI_ANN(Failure),
    STATUS_PENDING,
} Status;

typedef struct TI_ROOT {
    struct {
        int anon_x;
        int anon_y;
    };
    union {
        int as_int;
        float as_float;
    };
} TestAnonymous;

typedef struct {
    int inner_field;
} Inner;

typedef struct TI_ROOT {
    Inner inner;
    struct {
        Inner nested_inner;
    } nested;
} TestNested;

typedef struct TI_ROOT {
    const int const_member;
    volatile int volatile_member;
    const volatile int const_volatile_member;
} TestMemberQualifiers;

typedef struct TI_ROOT {
    int id;
    struct {
        char name[64] TI_ANN(CStr);
        struct {
            int x;
            int y;
            int z;
        } position;
    } data;
    enum { TYPE_A, TYPE_B, TYPE_C } type;
} TestComplex;

typedef struct TI_ROOT {
    void* void_ptr;
} TestVoidPtr;

#endif  // TEST_TYPES_H_
