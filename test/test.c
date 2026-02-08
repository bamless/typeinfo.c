#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#undef __STRICT_ANSI__
#define CTEST_MAIN
#define CTEST_SEGFAULT
#define CTEST_COLOR_OK
#include "ctest.h"
#include "test_types.h"
#include "test_types_typeinfo.h"
#include "typeinfo.h"

static bool has_annotation(char** annotations, const char* expected) {
    if(!annotations) return false;
    for(char** ann = annotations; *ann != NULL; ann++) {
        if(strcmp(*ann, expected) == 0) {
            return true;
        }
    }
    return false;
}

static size_t count_annotations(char** annotations) {
    if(!annotations) return 0;
    size_t count = 0;
    for(char** ann = annotations; *ann != NULL; ann++) {
        count++;
    }
    return count;
}

static Type_Info_Member* find_member(Type_Info_Struct* s, const char* name) {
    for(size_t i = 0; i < s->members_count; i++) {
        if(strcmp(s->members[i].name, name) == 0) {
            return &s->members[i];
        }
    }
    return NULL;
}

static Type_Info_Member* find_union_member(Type_Info_Union* u, const char* name) {
    for(size_t i = 0; i < u->members_count; i++) {
        if(strcmp(u->members[i].name, name) == 0) {
            return &u->members[i];
        }
    }
    return NULL;
}

static Type_Info_Enum_Value* find_enum_value(Type_Info_Enum* e, const char* name) {
    for(size_t i = 0; i < e->values_count; i++) {
        if(strcmp(e->values[i].name, name) == 0) {
            return &e->values[i];
        }
    }
    return NULL;
}

// ==============================================================================
// Basic Type Tests
// ==============================================================================

CTEST(basic_types, test_integers_struct_exists) {
    ASSERT_NOT_NULL(&typeinfo_TestIntegers);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, typeinfo_TestIntegers.base.tag);
    ASSERT_EQUAL(sizeof(TestIntegers), typeinfo_TestIntegers.base.size);
}

CTEST(basic_types, test_integers_member_count) {
    ASSERT_EQUAL(8, typeinfo_TestIntegers.members_count);
}

CTEST(basic_types, test_integers_i8_signed) {
    Type_Info_Member* member = find_member(&typeinfo_TestIntegers, "i8");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_INTEGER, member->type->tag);
    Type_Info_Integer* int_type = (Type_Info_Integer*)member->type;
    ASSERT_TRUE(int_type->is_signed);
    ASSERT_EQUAL(sizeof(int8_t), int_type->base.size);
}

CTEST(basic_types, test_integers_u8_unsigned) {
    Type_Info_Member* member = find_member(&typeinfo_TestIntegers, "u8");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_INTEGER, member->type->tag);
    Type_Info_Integer* int_type = (Type_Info_Integer*)member->type;
    ASSERT_FALSE(int_type->is_signed);
    ASSERT_EQUAL(sizeof(uint8_t), int_type->base.size);
}

CTEST(basic_types, test_integers_i32_signed) {
    Type_Info_Member* member = find_member(&typeinfo_TestIntegers, "i32");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_INTEGER, member->type->tag);
    Type_Info_Integer* int_type = (Type_Info_Integer*)member->type;
    ASSERT_TRUE(int_type->is_signed);
    ASSERT_EQUAL(sizeof(int32_t), int_type->base.size);
}

CTEST(basic_types, test_floats_struct_exists) {
    ASSERT_NOT_NULL(&typeinfo_TestFloats);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, typeinfo_TestFloats.base.tag);
    ASSERT_EQUAL(sizeof(TestFloats), typeinfo_TestFloats.base.size);
}

CTEST(basic_types, test_floats_member_count) {
    ASSERT_EQUAL(2, typeinfo_TestFloats.members_count);
}

CTEST(basic_types, test_float_type) {
    Type_Info_Member* member = find_member(&typeinfo_TestFloats, "f");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_FLOAT, member->type->tag);
    ASSERT_EQUAL(sizeof(float), member->type->size);
}

CTEST(basic_types, test_double_type) {
    Type_Info_Member* member = find_member(&typeinfo_TestFloats, "d");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_FLOAT, member->type->tag);
    ASSERT_EQUAL(sizeof(double), member->type->size);
}

// ==============================================================================
// Pointer Type Tests
// ==============================================================================

CTEST(pointer_types, test_pointers_struct_exists) {
    ASSERT_NOT_NULL(&typeinfo_TestPointers);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, typeinfo_TestPointers.base.tag);
}

CTEST(pointer_types, test_simple_pointer) {
    Type_Info_Member* member = find_member(&typeinfo_TestPointers, "ptr");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_POINTER, member->type->tag);
    Type_Info_Pointer* ptr_type = (Type_Info_Pointer*)member->type;
    ASSERT_EQUAL(sizeof(void*), ptr_type->base.size);
    ASSERT_NOT_NULL(ptr_type->pointer_to);
    ASSERT_EQUAL(TYPE_TAG_INTEGER, ptr_type->pointer_to->tag);
}

CTEST(pointer_types, test_const_pointer) {
    Type_Info_Member* member = find_member(&typeinfo_TestPointers, "const_ptr");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_POINTER, member->type->tag);
    Type_Info_Pointer* ptr_type = (Type_Info_Pointer*)member->type;
    ASSERT_TRUE(ptr_type->qualifier_flags & TYPE_INFO_QUALIFIER_CONST);
}

CTEST(pointer_types, test_volatile_pointer) {
    Type_Info_Member* member = find_member(&typeinfo_TestPointers, "volatile_ptr");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_POINTER, member->type->tag);
    Type_Info_Pointer* ptr_type = (Type_Info_Pointer*)member->type;
    ASSERT_TRUE(ptr_type->qualifier_flags & TYPE_INFO_QUALIFIER_VOLATILE);
}

CTEST(pointer_types, test_const_volatile_pointer) {
    Type_Info_Member* member = find_member(&typeinfo_TestPointers, "const_volatile_ptr");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_POINTER, member->type->tag);
    Type_Info_Pointer* ptr_type = (Type_Info_Pointer*)member->type;
    ASSERT_TRUE(ptr_type->qualifier_flags & TYPE_INFO_QUALIFIER_CONST);
    ASSERT_TRUE(ptr_type->qualifier_flags & TYPE_INFO_QUALIFIER_VOLATILE);
}

// ==============================================================================
// Array Type Tests
// ==============================================================================

CTEST(array_types, test_arrays_struct_exists) {
    ASSERT_NOT_NULL(&typeinfo_TestArrays);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, typeinfo_TestArrays.base.tag);
}

CTEST(array_types, test_simple_array) {
    Type_Info_Member* member = find_member(&typeinfo_TestArrays, "arr");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_ARRAY, member->type->tag);
    Type_Info_Array* arr_type = (Type_Info_Array*)member->type;
    ASSERT_EQUAL(10, arr_type->num_elements);
    ASSERT_NOT_NULL(arr_type->element_type);
    ASSERT_EQUAL(TYPE_TAG_INTEGER, arr_type->element_type->tag);
}

CTEST(array_types, test_string_array_with_annotation) {
    Type_Info_Member* member = find_member(&typeinfo_TestArrays, "str");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_ARRAY, member->type->tag);
    Type_Info_Array* arr_type = (Type_Info_Array*)member->type;
    ASSERT_EQUAL(256, arr_type->num_elements);
    ASSERT_TRUE(has_annotation(member->annotations, "CStr"));
}

CTEST(array_types, test_multidimensional_array) {
    Type_Info_Member* member = find_member(&typeinfo_TestArrays, "matrix");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_ARRAY, member->type->tag);
    Type_Info_Array* arr_type = (Type_Info_Array*)member->type;
    ASSERT_EQUAL(3, arr_type->num_elements);
    ASSERT_EQUAL(TYPE_TAG_ARRAY, arr_type->element_type->tag);
    Type_Info_Array* inner_arr = (Type_Info_Array*)arr_type->element_type;
    ASSERT_EQUAL(4, inner_arr->num_elements);
    ASSERT_EQUAL(TYPE_TAG_FLOAT, inner_arr->element_type->tag);
}

// ==============================================================================
// Struct Type Tests
// ==============================================================================

CTEST(struct_types, test_point_struct_annotation) {
    ASSERT_NOT_NULL(&typeinfo_Point);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, typeinfo_Point.base.tag);
    ASSERT_TRUE(has_annotation(typeinfo_Point.annotations, "StructAnnotation"));
}

CTEST(struct_types, test_point_members) {
    ASSERT_EQUAL(2, typeinfo_Point.members_count);
    Type_Info_Member* x = find_member(&typeinfo_Point, "x");
    Type_Info_Member* y = find_member(&typeinfo_Point, "y");
    ASSERT_NOT_NULL(x);
    ASSERT_NOT_NULL(y);
    ASSERT_TRUE(has_annotation(x->annotations, "XCoord"));
    ASSERT_TRUE(has_annotation(y->annotations, "YCoord"));
}

CTEST(struct_types, test_nested_struct) {
    Type_Info_Member* member = find_member(&typeinfo_TestStructs, "point");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, member->type->tag);
    Type_Info_Struct* point_type = (Type_Info_Struct*)member->type;
    ASSERT_STR("Point", point_type->name);
}

CTEST(struct_types, test_struct_pointer) {
    Type_Info_Member* member = find_member(&typeinfo_TestStructs, "point_ptr");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_POINTER, member->type->tag);
    Type_Info_Pointer* ptr_type = (Type_Info_Pointer*)member->type;
    ASSERT_EQUAL(TYPE_TAG_STRUCT, ptr_type->pointer_to->tag);
}

// ==============================================================================
// Union Type Tests
// ==============================================================================

CTEST(union_types, test_union_exists) {
    ASSERT_NOT_NULL(&typeinfo_TestUnion);
    ASSERT_EQUAL(TYPE_TAG_UNION, typeinfo_TestUnion.base.tag);
    ASSERT_TRUE(has_annotation(typeinfo_TestUnion.annotations, "UnionAnnotation"));
}

CTEST(union_types, test_union_members) {
    ASSERT_EQUAL(3, typeinfo_TestUnion.members_count);
    Type_Info_Member* i = find_union_member(&typeinfo_TestUnion, "i");
    Type_Info_Member* f = find_union_member(&typeinfo_TestUnion, "f");
    Type_Info_Member* bytes = find_union_member(&typeinfo_TestUnion, "bytes");
    ASSERT_NOT_NULL(i);
    ASSERT_NOT_NULL(f);
    ASSERT_NOT_NULL(bytes);
    ASSERT_EQUAL(TYPE_TAG_INTEGER, i->type->tag);
    ASSERT_EQUAL(TYPE_TAG_FLOAT, f->type->tag);
    ASSERT_EQUAL(TYPE_TAG_ARRAY, bytes->type->tag);
}

CTEST(union_types, test_union_size) {
    // Union size should be at least as large as its largest member
    ASSERT_GE_U(typeinfo_TestUnion.base.size, sizeof(int));
    ASSERT_GE_U(typeinfo_TestUnion.base.size, sizeof(float));
    ASSERT_GE_U(typeinfo_TestUnion.base.size, 4);
}

// ==============================================================================
// Enum Type Tests
// ==============================================================================

CTEST(enum_types, test_enum_exists) {
    ASSERT_NOT_NULL(&typeinfo_Status);
    ASSERT_EQUAL(TYPE_TAG_ENUM, typeinfo_Status.base.tag);
}

CTEST(enum_types, test_enum_values_count) {
    ASSERT_EQUAL(3, typeinfo_Status.values_count);
}

CTEST(enum_types, test_enum_value_names) {
    Type_Info_Enum_Value* ok = find_enum_value(&typeinfo_Status, "STATUS_OK");
    Type_Info_Enum_Value* error = find_enum_value(&typeinfo_Status, "STATUS_ERROR");
    Type_Info_Enum_Value* pending = find_enum_value(&typeinfo_Status, "STATUS_PENDING");
    ASSERT_NOT_NULL(ok);
    ASSERT_NOT_NULL(error);
    ASSERT_NOT_NULL(pending);
}

CTEST(enum_types, test_enum_value_annotations) {
    Type_Info_Enum_Value* ok = find_enum_value(&typeinfo_Status, "STATUS_OK");
    Type_Info_Enum_Value* error = find_enum_value(&typeinfo_Status, "STATUS_ERROR");
    ASSERT_NOT_NULL(ok);
    ASSERT_NOT_NULL(error);
    ASSERT_TRUE(has_annotation(ok->annotations, "Success"));
    ASSERT_TRUE(has_annotation(error->annotations, "Failure"));
}

CTEST(enum_types, test_enum_values) {
    Type_Info_Enum_Value* ok = find_enum_value(&typeinfo_Status, "STATUS_OK");
    Type_Info_Enum_Value* error = find_enum_value(&typeinfo_Status, "STATUS_ERROR");
    Type_Info_Enum_Value* pending = find_enum_value(&typeinfo_Status, "STATUS_PENDING");
    ASSERT_EQUAL(STATUS_OK, ok->value);
    ASSERT_EQUAL(STATUS_ERROR, error->value);
    ASSERT_EQUAL(STATUS_PENDING, pending->value);
}

// ==============================================================================
// Anonymous Type Tests
// ==============================================================================

CTEST(anonymous_types, test_anonymous_struct_exists) {
    ASSERT_NOT_NULL(&typeinfo_TestAnonymous);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, typeinfo_TestAnonymous.base.tag);
}

CTEST(anonymous_types, test_anonymous_struct_members) {
    // Anonymous struct members are represented as nested anonymous structs
    // The TestAnonymous struct should have 2 members (anonymous struct and anonymous union)
    ASSERT_EQUAL(2, typeinfo_TestAnonymous.members_count);

    // First member should be an anonymous struct
    Type_Info_Member* anon_struct = &typeinfo_TestAnonymous.members[0];
    ASSERT_EQUAL(TYPE_TAG_STRUCT, anon_struct->type->tag);
    Type_Info_Struct* anon_struct_type = (Type_Info_Struct*)anon_struct->type;
    ASSERT_STR("", anon_struct_type->name);  // Anonymous structs have empty names
}

CTEST(anonymous_types, test_anonymous_union_members) {
    // Anonymous union members are represented as nested anonymous unions
    // Second member should be an anonymous union
    Type_Info_Member* anon_union = &typeinfo_TestAnonymous.members[1];
    ASSERT_EQUAL(TYPE_TAG_UNION, anon_union->type->tag);
    Type_Info_Union* anon_union_type = (Type_Info_Union*)anon_union->type;
    ASSERT_STR("", anon_union_type->name);  // Anonymous unions have empty names
}

// ==============================================================================
// Nested Type Tests
// ==============================================================================

CTEST(nested_types, test_nested_struct_exists) {
    ASSERT_NOT_NULL(&typeinfo_TestNested);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, typeinfo_TestNested.base.tag);
}

CTEST(nested_types, test_inner_struct_member) {
    Type_Info_Member* inner = find_member(&typeinfo_TestNested, "inner");
    ASSERT_NOT_NULL(inner);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, inner->type->tag);
    Type_Info_Struct* inner_type = (Type_Info_Struct*)inner->type;
    ASSERT_STR("Inner", inner_type->name);
}

CTEST(nested_types, test_nested_anonymous_struct) {
    Type_Info_Member* nested = find_member(&typeinfo_TestNested, "nested");
    ASSERT_NOT_NULL(nested);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, nested->type->tag);
}

// ==============================================================================
// Qualifier Tests
// ==============================================================================

CTEST(qualifiers, test_const_member) {
    Type_Info_Member* member = find_member(&typeinfo_TestMemberQualifiers, "const_member");
    ASSERT_NOT_NULL(member);
    ASSERT_TRUE(member->qualifier_flags & TYPE_INFO_QUALIFIER_CONST);
}

CTEST(qualifiers, test_volatile_member) {
    Type_Info_Member* member = find_member(&typeinfo_TestMemberQualifiers, "volatile_member");
    ASSERT_NOT_NULL(member);
    ASSERT_TRUE(member->qualifier_flags & TYPE_INFO_QUALIFIER_VOLATILE);
}

CTEST(qualifiers, test_const_volatile_member) {
    Type_Info_Member* member = find_member(&typeinfo_TestMemberQualifiers, "const_volatile_member");
    ASSERT_NOT_NULL(member);
    ASSERT_TRUE(member->qualifier_flags & TYPE_INFO_QUALIFIER_CONST);
    ASSERT_TRUE(member->qualifier_flags & TYPE_INFO_QUALIFIER_VOLATILE);
}

// ==============================================================================
// Complex Type Tests
// ==============================================================================

CTEST(complex_types, test_complex_struct_exists) {
    ASSERT_NOT_NULL(&typeinfo_TestComplex);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, typeinfo_TestComplex.base.tag);
}

CTEST(complex_types, test_complex_nested_anonymous) {
    Type_Info_Member* data = find_member(&typeinfo_TestComplex, "data");
    ASSERT_NOT_NULL(data);
    ASSERT_EQUAL(TYPE_TAG_STRUCT, data->type->tag);
}

CTEST(complex_types, test_complex_anonymous_enum) {
    Type_Info_Member* type = find_member(&typeinfo_TestComplex, "type");
    ASSERT_NOT_NULL(type);
    ASSERT_EQUAL(TYPE_TAG_ENUM, type->type->tag);
}

// ==============================================================================
// Void Pointer Tests
// ==============================================================================

CTEST(void_types, test_void_pointer) {
    Type_Info_Member* member = find_member(&typeinfo_TestVoidPtr, "void_ptr");
    ASSERT_NOT_NULL(member);
    ASSERT_EQUAL(TYPE_TAG_POINTER, member->type->tag);
    Type_Info_Pointer* ptr_type = (Type_Info_Pointer*)member->type;
    ASSERT_NOT_NULL(ptr_type->pointer_to);
    ASSERT_EQUAL(TYPE_TAG_VOID, ptr_type->pointer_to->tag);
}

// ==============================================================================
// Size and Alignment Tests
// ==============================================================================

CTEST(size_alignment, test_struct_sizes_match) {
    ASSERT_EQUAL(sizeof(TestIntegers), typeinfo_TestIntegers.base.size);
    ASSERT_EQUAL(sizeof(TestFloats), typeinfo_TestFloats.base.size);
    ASSERT_EQUAL(sizeof(TestPointers), typeinfo_TestPointers.base.size);
    ASSERT_EQUAL(sizeof(TestArrays), typeinfo_TestArrays.base.size);
}

CTEST(size_alignment, test_alignment_values) {
    ASSERT_EQUAL(TYPEINFO_ALIGNOF(TestIntegers), typeinfo_TestIntegers.base.alignment);
    ASSERT_EQUAL(TYPEINFO_ALIGNOF(TestFloats), typeinfo_TestFloats.base.alignment);
    ASSERT_EQUAL(TYPEINFO_ALIGNOF(TestPointers), typeinfo_TestPointers.base.alignment);
}

CTEST(size_alignment, test_member_offsets) {
    Type_Info_Member* i8 = find_member(&typeinfo_TestIntegers, "i8");
    Type_Info_Member* u8 = find_member(&typeinfo_TestIntegers, "u8");
    ASSERT_NOT_NULL(i8);
    ASSERT_NOT_NULL(u8);
    ASSERT_EQUAL(offsetof(TestIntegers, i8), i8->offset);
    ASSERT_EQUAL(offsetof(TestIntegers, u8), u8->offset);
}

int main(int argc, const char** argv) {
    return ctest_main(argc, argv);
}
