#ifndef TYPEINFO_H_
#define TYPEINFO_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef RUNNING_TYPEINFO_METAPROGRAM
    #define TI_ROOT   __attribute__((annotate("__TypeInfoRoot")))
    #define TI_ANN(x) __attribute__((annotate(#x)))
#else
    #define TI_ROOT
    #define TI_ANN(x)
#endif

#if __STDC_VERSION__ >= 201112L
    #define TYPEINFO_ALIGNOF(T) _Alignof(T)
#elif defined(__GNUC__) || defined(__clang__)
    #define TYPEINFO_ALIGNOF(T) __alignof__(T)
#elif defined(_MSC_VER)
    #define TYPEINFO_ALIGNOF(T) __alignof(T)
#else
    #error "No alignof support detected for this compiler"
#endif

typedef enum {
    TYPE_TAG_VOID,
    TYPE_TAG_INTEGER,
    TYPE_TAG_FLOAT,
    TYPE_TAG_POINTER,
    TYPE_TAG_ARRAY,
    TYPE_TAG_STRUCT,
    TYPE_TAG_UNION,
    TYPE_TAG_ENUM,
} Type_Info_Tag;

typedef enum {
    TYPE_INFO_QUALIFIER_NONE = 0,
    TYPE_INFO_QUALIFIER_CONST = 1 << 0,
    TYPE_INFO_QUALIFIER_VOLATILE = 1 << 1,
    TYPE_INFO_QUALIFIER_RESTRICT = 1 << 2,
} Type_Info_Qualifier;

typedef struct {
    Type_Info_Tag tag;
    size_t size;
    size_t alignment;
} Type_Info;

typedef struct {
    Type_Info base;
} Type_Info_Void;

typedef struct {
    Type_Info base;
    bool is_signed;
} Type_Info_Integer;

typedef struct {
    Type_Info base;
} Type_Info_Float;

typedef struct {
    Type_Info base;
    Type_Info* pointer_to;     // Inner type this pointer points-to
    uint32_t qualifier_flags;  // Qualifiers applied to the pointer_to type; Bitmask of
                               // `Type_Info_Qualifier`
} Type_Info_Pointer;

typedef struct {
    Type_Info base;
    size_t num_elements;
    Type_Info* element_type;
} Type_Info_Array;

typedef struct {
    char** annotations;
    const char* name;
    size_t offset;
    Type_Info* type;
    uint32_t qualifier_flags;  // Qualifiers applied to this struct or union member; Bitmask of
                               // `Type_Info_Qualifier`
} Type_Info_Member;

typedef struct {
    Type_Info base;
    char** annotations;
    const char* name;
    Type_Info_Member* members;
    size_t members_count;
} Type_Info_Struct;

typedef struct {
    Type_Info base;
    char** annotations;
    const char* name;
    Type_Info_Member* members;
    size_t members_count;
} Type_Info_Union;

typedef struct {
    char** annotations;
    const char* name;
    long long value;
} Type_Info_Enum_Value;

typedef struct {
    Type_Info base;
    char** annotations;
    const char* name;
    Type_Info_Enum_Value* values;
    size_t values_count;
} Type_Info_Enum;

#endif  // TYPEINFO_H_
