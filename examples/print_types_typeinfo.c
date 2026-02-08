#include "typeinfo.h"
#include "print_types_typeinfo.h"

Type_Info_Void typeinfo_void = {{ TYPE_TAG_VOID, 0, 0 }};
Type_Info_Integer typeinfo_bool = {{ TYPE_TAG_INTEGER, sizeof(_Bool), TYPEINFO_ALIGNOF(_Bool) }, 0};
Type_Info_Integer typeinfo_char = {{ TYPE_TAG_INTEGER, sizeof(char), TYPEINFO_ALIGNOF(char) }, (char)-1 < 0};
Type_Info_Integer typeinfo_signed_char = {{ TYPE_TAG_INTEGER, sizeof(signed char), TYPEINFO_ALIGNOF(signed char) }, 1};
Type_Info_Integer typeinfo_unsigned_char = {{ TYPE_TAG_INTEGER, sizeof(unsigned char), TYPEINFO_ALIGNOF(unsigned char) }, 0};
Type_Info_Integer typeinfo_short = {{ TYPE_TAG_INTEGER, sizeof(short), TYPEINFO_ALIGNOF(short) }, 1};
Type_Info_Integer typeinfo_unsigned_short = {{ TYPE_TAG_INTEGER, sizeof(unsigned short), TYPEINFO_ALIGNOF(unsigned short) }, 0};
Type_Info_Integer typeinfo_int = {{ TYPE_TAG_INTEGER, sizeof(int), TYPEINFO_ALIGNOF(int) }, 1};
Type_Info_Integer typeinfo_unsigned_int = {{ TYPE_TAG_INTEGER, sizeof(unsigned int), TYPEINFO_ALIGNOF(unsigned int) }, 0};
Type_Info_Integer typeinfo_long = {{ TYPE_TAG_INTEGER, sizeof(long), TYPEINFO_ALIGNOF(long) }, 1};
Type_Info_Integer typeinfo_unsigned_long = {{ TYPE_TAG_INTEGER, sizeof(unsigned long), TYPEINFO_ALIGNOF(unsigned long) }, 0};
Type_Info_Integer typeinfo_long_long = {{ TYPE_TAG_INTEGER, sizeof(long long), TYPEINFO_ALIGNOF(long long) }, 1};
Type_Info_Integer typeinfo_unsigned_long_long = {{ TYPE_TAG_INTEGER, sizeof(unsigned long long), TYPEINFO_ALIGNOF(unsigned long long) }, 0};
Type_Info_Float typeinfo_float = {{ TYPE_TAG_FLOAT, sizeof(float), TYPEINFO_ALIGNOF(float) }};
Type_Info_Float typeinfo_double = {{ TYPE_TAG_FLOAT, sizeof(double), TYPEINFO_ALIGNOF(double) }};
Type_Info_Float typeinfo_long_double = {{ TYPE_TAG_FLOAT, sizeof(long double), TYPEINFO_ALIGNOF(long double) }};

// struct Foo
// examples/print_types.h:24:9
static Type_Info_Member members_Foo[] = {
  { (char*[]){ "CStr", NULL }, "name", 0, (Type_Info*)&(Type_Info_Array){{TYPE_TAG_ARRAY, 256, 1}, 256, (Type_Info*)&typeinfo_char }, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "test", 256, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "bar", 264, (Type_Info*)&(Type_Info_Pointer){{TYPE_TAG_POINTER, sizeof(void*), 8}, (Type_Info*)&typeinfo_Bar, TYPE_INFO_QUALIFIER_NONE }, TYPE_INFO_QUALIFIER_NONE },
};
Type_Info_Struct typeinfo_Foo = {
  { TYPE_TAG_STRUCT, 272, 8 },
  (char*[]){ NULL },
  "Foo",
  members_Foo,
  sizeof(members_Foo)/sizeof(*members_Foo)
};

// enum Color
// examples/print_types.h:30:9
static Type_Info_Enum_Value values_Color[] = {
  { (char*[]){ "Primary", "Secondary", NULL }, "COLOR_RED", 0 },
  { (char*[]){ "Primary", NULL }, "COLOR_GREEN", 1 },
  { (char*[]){ "Primary", NULL }, "COLOR_BLUE", 2 },
  { (char*[]){ NULL }, "COLOR_YELLOW", 3 },
  { (char*[]){ NULL }, "COLOR_CYAN", 4 },
  { (char*[]){ NULL }, "COLOR_MAGENTA", 5 },
};
Type_Info_Enum typeinfo_Color = {
  { TYPE_TAG_ENUM, 4, 4 },
  (char*[]){ NULL },
  "Color",
  values_Color,
  sizeof(values_Color)/sizeof(*values_Color)
};

// union TestUnion
// examples/print_types.h:39:9
static Type_Info_Member members_TestUnion[] = {
  { (char*[]){ NULL }, "i", 0, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "f", 0, (Type_Info*)&typeinfo_float, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "c", 0, (Type_Info*)&(Type_Info_Array){{TYPE_TAG_ARRAY, 4, 1}, 4, (Type_Info*)&typeinfo_char }, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "anon_struct", 0, (Type_Info*)&(Type_Info_Struct){{TYPE_TAG_STRUCT, 8, 4}, (char*[]){ NULL }, "", (Type_Info_Member[]){
    { (char*[]){ NULL }, "x", 0, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
    { (char*[]){ NULL }, "y", 4, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
  }, 2 }, TYPE_INFO_QUALIFIER_NONE },
};
Type_Info_Union typeinfo_TestUnion = {
  { TYPE_TAG_UNION, 8, 4 },
  (char*[]){ NULL },
  "TestUnion",
  members_TestUnion,
  sizeof(members_TestUnion)/sizeof(*members_TestUnion)
};

// struct TestAnonymousEnum
// examples/print_types.h:49:9
static Type_Info_Member members_TestAnonymousEnum[] = {
  { (char*[]){ NULL }, "anon_enum_field", 0, (Type_Info*)&(Type_Info_Enum){{TYPE_TAG_ENUM, 4, 4}, (char*[]){ NULL }, "", (Type_Info_Enum_Value[]){
    { (char*[]){ NULL }, "ANON_A", 0 },
    { (char*[]){ NULL }, "ANON_B", 1 },
    { (char*[]){ NULL }, "ANON_C", 2 },
  }, 3 }, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "other_field", 4, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
};
Type_Info_Struct typeinfo_TestAnonymousEnum = {
  { TYPE_TAG_STRUCT, 8, 4 },
  (char*[]){ NULL },
  "TestAnonymousEnum",
  members_TestAnonymousEnum,
  sizeof(members_TestAnonymousEnum)/sizeof(*members_TestAnonymousEnum)
};

// struct TestUnnamedAnonymous
// examples/print_types.h:54:9
static Type_Info_Member members_TestUnnamedAnonymous[] = {
  { (char*[]){ NULL }, "before", 0, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "", 4, (Type_Info*)&(Type_Info_Struct){{TYPE_TAG_STRUCT, 4, 4}, (char*[]){ NULL }, "", (Type_Info_Member[]){
    { (char*[]){ NULL }, "", 0, (Type_Info*)&(Type_Info_Union){{TYPE_TAG_UNION, 4, 4}, (char*[]){ NULL }, "", (Type_Info_Member[]){
      { (char*[]){ "X1", NULL }, "x", 0, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
      { (char*[]){ "Y1", NULL }, "y", 0, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
    }, 2 }, TYPE_INFO_QUALIFIER_NONE },
  }, 1 }, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "after", 8, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
};
Type_Info_Struct typeinfo_TestUnnamedAnonymous = {
  { TYPE_TAG_STRUCT, 12, 4 },
  (char*[]){ NULL },
  "TestUnnamedAnonymous",
  members_TestUnnamedAnonymous,
  sizeof(members_TestUnnamedAnonymous)/sizeof(*members_TestUnnamedAnonymous)
};

// struct TestQualifiers
// examples/print_types.h:65:9
static Type_Info_Member members_TestQualifiers[] = {
  { (char*[]){ NULL }, "ci", 0, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_CONST },
  { (char*[]){ NULL }, "vi", 4, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_VOLATILE },
  { (char*[]){ NULL }, "cstr_ptr", 8, (Type_Info*)&(Type_Info_Pointer){{TYPE_TAG_POINTER, sizeof(void*), 8}, (Type_Info*)&typeinfo_char, TYPE_INFO_QUALIFIER_CONST }, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "const_ptr", 16, (Type_Info*)&(Type_Info_Pointer){{TYPE_TAG_POINTER, sizeof(void*), 8}, (Type_Info*)&typeinfo_char, TYPE_INFO_QUALIFIER_NONE }, TYPE_INFO_QUALIFIER_CONST },
  { (char*[]){ NULL }, "both", 24, (Type_Info*)&(Type_Info_Pointer){{TYPE_TAG_POINTER, sizeof(void*), 8}, (Type_Info*)&typeinfo_char, TYPE_INFO_QUALIFIER_CONST }, TYPE_INFO_QUALIFIER_CONST },
};
Type_Info_Struct typeinfo_TestQualifiers = {
  { TYPE_TAG_STRUCT, 32, 8 },
  (char*[]){ NULL },
  "TestQualifiers",
  members_TestQualifiers,
  sizeof(members_TestQualifiers)/sizeof(*members_TestQualifiers)
};

// struct Bar
// examples/print_types.h:13:9
static Type_Info_Member members_Bar[] = {
  { (char*[]){ NULL }, "x", 0, (Type_Info*)&typeinfo_unsigned_long, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "y", 8, (Type_Info*)&typeinfo_unsigned_long, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "baz", 16, (Type_Info*)&(Type_Info_Array){{TYPE_TAG_ARRAY, 48, 8}, 3, (Type_Info*)&typeinfo_Baz }, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "anon", 64, (Type_Info*)&(Type_Info_Struct){{TYPE_TAG_STRUCT, 4, 4}, (char*[]){ NULL }, "", (Type_Info_Member[]){
    { (char*[]){ NULL }, "anon2", 0, (Type_Info*)&(Type_Info_Struct){{TYPE_TAG_STRUCT, 4, 4}, (char*[]){ NULL }, "", (Type_Info_Member[]){
      { (char*[]){ NULL }, "x", 0, (Type_Info*)&typeinfo_int, TYPE_INFO_QUALIFIER_NONE },
    }, 1 }, TYPE_INFO_QUALIFIER_NONE },
  }, 1 }, TYPE_INFO_QUALIFIER_NONE },
};
Type_Info_Struct typeinfo_Bar = {
  { TYPE_TAG_STRUCT, 72, 8 },
  (char*[]){ NULL },
  "Bar",
  members_Bar,
  sizeof(members_Bar)/sizeof(*members_Bar)
};

// struct Baz
// examples/print_types.h:8:9
static Type_Info_Member members_Baz[] = {
  { (char*[]){ NULL }, "iptr", 0, (Type_Info*)&typeinfo_long, TYPE_INFO_QUALIFIER_NONE },
  { (char*[]){ NULL }, "ptr", 8, (Type_Info*)&(Type_Info_Pointer){{TYPE_TAG_POINTER, sizeof(void*), 8}, (Type_Info*)&typeinfo_void, TYPE_INFO_QUALIFIER_NONE }, TYPE_INFO_QUALIFIER_NONE },
};
Type_Info_Struct typeinfo_Baz = {
  { TYPE_TAG_STRUCT, 16, 8 },
  (char*[]){ "BazAnnotation", NULL },
  "Baz",
  members_Baz,
  sizeof(members_Baz)/sizeof(*members_Baz)
};

