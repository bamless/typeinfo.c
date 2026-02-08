#include "print_types.h"

#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "print_types_typeinfo.h"
#include "typeinfo.h"

static void print_indent(int indent) {
    for(int i = 0; i < indent; i++) putchar(' ');
}

static bool is_valid_cstr_type(Type_Info* type) {
    if(!type) return false;
    Type_Info* type_inner;
    switch(type->tag) {
    case TYPE_TAG_POINTER:
        type_inner = ((Type_Info_Pointer*)type)->pointer_to;
        break;
    case TYPE_TAG_ARRAY:
        type_inner = ((Type_Info_Array*)type)->element_type;
        break;
    default:
        type_inner = NULL;
        break;
    }
    return type_inner == &typeinfo_char.base;
}

void print_value(const void* value, Type_Info* ti, int indent) {
    if(!ti) {
        printf("<null typeinfo>\n");
        return;
    }
    if(!value) {
        printf("<null value>\n");
        return;
    }

    switch(ti->tag) {
    case TYPE_TAG_INTEGER: {
        Type_Info_Integer* iinfo = (Type_Info_Integer*)ti;
        switch(ti->size) {
        case 1:
            if(iinfo->is_signed) printf("%" PRId8, *(int8_t*)value);
            else printf("%" PRIu8 "\n", *(uint8_t*)value);

            if(ti == &typeinfo_char.base) {
                char c = *(char*)value;
                if(isprint(c)) printf(" ('%c')", c);
            }

            printf("\n");
            break;
        case 2:
            if(iinfo->is_signed) printf("%" PRId16 "\n", *(int16_t*)value);
            else printf("%" PRIu16 "\n", *(uint16_t*)value);
            break;
        case 4:
            if(iinfo->is_signed) printf("%" PRId32 "\n", *(int32_t*)value);
            else printf("%" PRIu32 "\n", *(uint32_t*)value);
            break;
        case 8:
            if(iinfo->is_signed) printf("%" PRId64 "\n", *(int64_t*)value);
            else printf("%" PRIu64 "\n", *(uint64_t*)value);
            break;
        default:
            printf("<unknown integer size %zu>\n", ti->size);
        }
    } break;

    case TYPE_TAG_FLOAT: {
        if(ti->size == sizeof(float)) printf("%f\n", *(float*)value);
        else if(ti->size == sizeof(double)) printf("%lf\n", *(double*)value);
        else if(ti->size == sizeof(long double)) printf("%Lf\n", *(long double*)value);
        else printf("<unknown float size %zu>\n", ti->size);
    } break;

    case TYPE_TAG_POINTER: {
        Type_Info_Pointer* ptinfo = (Type_Info_Pointer*)ti;
        void* p = *(void**)value;
        if(!p) {
            printf("NULL\n");
            break;
        }

        printf("%p", p);

        // If we have base typeinfo, recurse once
        if(ptinfo->pointer_to && ptinfo->pointer_to->tag != TYPE_TAG_POINTER) {
            printf(" -> ");
            print_value(p, ptinfo->pointer_to, indent);
        } else {
            printf("\n");
        }
    } break;

    case TYPE_TAG_ARRAY: {
        Type_Info_Array* ainfo = (Type_Info_Array*)ti;
        printf("[\n");

        Type_Info* elem_ti = ainfo->element_type;
        for(size_t i = 0; i < ainfo->num_elements; ++i) {
            void* elem_ptr = (char*)value + i * elem_ti->size;
            print_indent(indent + 2);
            print_value(elem_ptr, elem_ti, indent + 2);
        }

        print_indent(indent);
        printf("]\n");
    } break;

    case TYPE_TAG_STRUCT: {
        Type_Info_Struct* sinfo = (Type_Info_Struct*)ti;
        printf("struct %s {\n", sinfo->name);
        for(size_t i = 0; i < sinfo->members_count; i++) {
            Type_Info_Member* m = &sinfo->members[i];

            print_indent(indent + 2);

            if(m->qualifier_flags & TYPE_INFO_QUALIFIER_CONST) printf("const ");
            if(m->qualifier_flags & TYPE_INFO_QUALIFIER_VOLATILE) printf("volatile ");
            if(m->qualifier_flags & TYPE_INFO_QUALIFIER_RESTRICT) printf("restrict ");
            if(strcmp(m->name, "") != 0) printf("%s = ", m->name);

            bool as_cstr = false;
            for(char** it = m->annotations; *it; it++) {
                if(strcmp(*it, "CStr") == 0) {
                    as_cstr = true;
                    break;
                }
            }

            void* field_ptr = (char*)value + m->offset;

            if(as_cstr && is_valid_cstr_type(m->type)) {
                Type_Info_Tag tag = m->type->tag;
                if(tag == TYPE_TAG_POINTER) {
                    printf("\"%s\"\n", *(char**)field_ptr);
                } else {
                    printf("\"%s\"\n", (char*)field_ptr);
                }
            } else {
                print_value(field_ptr, m->type, indent + 2);
            }
        }
        print_indent(indent);
        printf("}\n");
    } break;

    case TYPE_TAG_UNION: {
        Type_Info_Union* uinfo = (Type_Info_Union*)ti;
        printf("union %s {\n", uinfo->name);
        for(size_t i = 0; i < uinfo->members_count; i++) {
            Type_Info_Member* m = &uinfo->members[i];

            print_indent(indent + 2);

            if(m->qualifier_flags & TYPE_INFO_QUALIFIER_CONST) printf("const ");
            if(m->qualifier_flags & TYPE_INFO_QUALIFIER_VOLATILE) printf("volatile ");
            if(m->qualifier_flags & TYPE_INFO_QUALIFIER_RESTRICT) printf("restrict ");
            if(strcmp(m->name, "") != 0) printf("%s = ", m->name);

            // All union members start at offset 0
            print_value(value, m->type, indent + 2);
        }
        print_indent(indent);
        printf("}\n");
    } break;

    case TYPE_TAG_ENUM: {
        Type_Info_Enum* einfo = (Type_Info_Enum*)ti;
        long long enum_value;

        // Read the enum value based on its size
        switch(ti->size) {
        case 1:
            enum_value = *(int8_t*)value;
            break;
        case 2:
            enum_value = *(int16_t*)value;
            break;
        case 4:
            enum_value = *(int32_t*)value;
            break;
        case 8:
            enum_value = *(int64_t*)value;
            break;
        default:
            printf("<unknown enum size %zu>\n", ti->size);
            return;
        }

        // Find the matching enum name
        const char* name = NULL;
        for(size_t i = 0; i < einfo->values_count; i++) {
            if(einfo->values[i].value == enum_value) {
                name = einfo->values[i].name;
                break;
            }
        }

        if(name) {
            printf("enum %s { %s (%lld) }\n", einfo->name, name, enum_value);
        } else {
            printf("<unknown enum value %lld>\n", enum_value);
        }
    } break;

    case TYPE_TAG_VOID: {
        printf("void\n");
    } break;

    default: {
        printf("<unsupported type %d>\n", ti->tag);
    } break;
    }
}

#define print(v, T) print_value(&v, (Type_Info*)&typeinfo_##T, 0)

int main(void) {
    Foo foo = {
        "ciaone",
        2,
        &(Bar){
            5,
            5,
            {{0xff, (void*)0xfafa}, {0xf0, (void*)0xfafa}, {0xfff, NULL}},
            .anon = {{0}},
        },
    };
    print(foo, Foo);

    printf("\n");

    TestUnion u = {.f = 3.14f};
    print(u, TestUnion);

    printf("\n");

    Color color = COLOR_BLUE;
    printf("color = ");
    print(color, Color);

    printf("\n");

    TestAnonymousEnum anon_enum_test = {.anon_enum_field = ANON_B, .other_field = 42};
    print(anon_enum_test, TestAnonymousEnum);

    printf("\n");

    TestUnnamedAnonymous unnamed_test = {.before = 10, .x = 20, .after = 40};
    print(unnamed_test, TestUnnamedAnonymous);

    printf("\n");

    static char buf[] = "mutable";
    TestQualifiers qual_test = {
        .ci = 42,
        .vi = 7,
        .cstr_ptr = "hello",
        .const_ptr = buf,
        .both = "world",
    };
    print(qual_test, TestQualifiers);
}
