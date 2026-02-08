#include <assert.h>
#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXTLIB_IMPL
#include "extlib.h"

#define TYPE_INFO_ANNOTATION "__TypeInfoRoot"
#define RUNNING_METAPROGRAM  "-DRUNNING_TYPEINFO_METAPROGRAM"
#define INDENT               2

#define shift(argc, argv) ((argc)--, *(argv)++)
#define ctx_indent(ctx)   defer_loop((ctx)->indent += INDENT, (ctx)->indent -= INDENT)

typedef struct {
    const char* out;
    bool recursive;
    bool no_builtin_types;
    char** files;
    int count;
    struct {
        char** items;
        size_t size, capacity;
        void* allocator;
    } forwarded;
} Opts;

typedef struct {
    char** items;
    size_t size, capacity;
    void* allocator;
} Annotations;

typedef struct {
    char* key;
    bool value;
} Visited_Type_Entry;

typedef struct {
    Visited_Type_Entry* entries;
    size_t* hashes;
    size_t size, capacity;
    void* allocator;
} Visited_Types;

typedef struct {
    CXType* items;
    size_t size, capacity;
    void* allocator;
} Type_Queue;

typedef struct {
    FILE* header;
    FILE* source;
    int indent;
    Visited_Types* visited_types;
    Type_Queue* pending_types;
} Type_Info_Context;

static Opts opts;

static const char* builtin_symbol(enum CXTypeKind kind) {
    switch(kind) {
    case CXType_Bool:
        return "bool";
    case CXType_Char_S:
        return "char";  // plain char
    case CXType_Char_U:
        return "char";  // plain char
    case CXType_SChar:
        return "signed_char";
    case CXType_UChar:
        return "unsigned_char";
    case CXType_Short:
        return "short";
    case CXType_UShort:
        return "unsigned_short";
    case CXType_Int:
        return "int";
    case CXType_UInt:
        return "unsigned_int";
    case CXType_Long:
        return "long";
    case CXType_ULong:
        return "unsigned_long";
    case CXType_LongLong:
        return "long_long";
    case CXType_ULongLong:
        return "unsigned_long_long";
    case CXType_Float:
        return "float";
    case CXType_Double:
        return "double";
    case CXType_LongDouble:
        return "long_double";
    case CXType_Void:
        return "void";
    default:
        return NULL;
    }
}

// Emits a qualifier flags expression (e.g., "TYPE_INFO_QUALIFIER_CONST |
// TYPE_INFO_QUALIFIER_VOLATILE") to the output file. Emits "TYPE_INFO_QUALIFIER_NONE" when no
// qualifiers are present.
static void emit_qualifier_flags(FILE* out, CXType type) {
    int parts_count = 0;
    const char* parts[3];

    if(clang_isConstQualifiedType(type)) parts[parts_count++] = "TYPE_INFO_QUALIFIER_CONST";
    if(clang_isVolatileQualifiedType(type)) parts[parts_count++] = "TYPE_INFO_QUALIFIER_VOLATILE";
    if(clang_isRestrictQualifiedType(type)) parts[parts_count++] = "TYPE_INFO_QUALIFIER_RESTRICT";

    if(parts_count != 0) {
        for(int i = 0; i < parts_count; i++) {
            if(i > 0) fprintf(out, " | ");
            fprintf(out, "%s", parts[i]);
        }
    } else {
        fprintf(out, "TYPE_INFO_QUALIFIER_NONE");
    }
}

static void emit_builtin_decls(FILE* header) {
    fprintf(header,
            "extern Type_Info_Void typeinfo_void;\n"
            "extern Type_Info_Integer typeinfo_bool;\n"
            "extern Type_Info_Integer typeinfo_char;\n"
            "extern Type_Info_Integer typeinfo_signed_char;\n"
            "extern Type_Info_Integer typeinfo_unsigned_char;\n"
            "extern Type_Info_Integer typeinfo_short;\n"
            "extern Type_Info_Integer typeinfo_unsigned_short;\n"
            "extern Type_Info_Integer typeinfo_int;\n"
            "extern Type_Info_Integer typeinfo_unsigned_int;\n"
            "extern Type_Info_Integer typeinfo_long;\n"
            "extern Type_Info_Integer typeinfo_unsigned_long;\n"
            "extern Type_Info_Integer typeinfo_long_long;\n"
            "extern Type_Info_Integer typeinfo_unsigned_long_long;\n"
            "extern Type_Info_Float typeinfo_float;\n"
            "extern Type_Info_Float typeinfo_double;\n"
            "extern Type_Info_Float typeinfo_long_double;\n\n");
}

static void emit_builtin_defs(FILE* source) {
    fprintf(source,
            "Type_Info_Void typeinfo_void = {{ TYPE_TAG_VOID, 0, 0 }};\n"
            "Type_Info_Integer typeinfo_bool = {{ TYPE_TAG_INTEGER, sizeof(_Bool), "
            "TYPEINFO_ALIGNOF(_Bool) }, 0};\n"
            "Type_Info_Integer typeinfo_char = {{ TYPE_TAG_INTEGER, sizeof(char), "
            "TYPEINFO_ALIGNOF(char) }, (char)-1 < 0};\n"
            "Type_Info_Integer typeinfo_signed_char = {{ TYPE_TAG_INTEGER, sizeof(signed char), "
            "TYPEINFO_ALIGNOF(signed char) }, 1};\n"
            "Type_Info_Integer typeinfo_unsigned_char = {{ TYPE_TAG_INTEGER, sizeof(unsigned "
            "char), TYPEINFO_ALIGNOF(unsigned char) }, 0};\n"
            "Type_Info_Integer typeinfo_short = {{ TYPE_TAG_INTEGER, sizeof(short), "
            "TYPEINFO_ALIGNOF(short) }, 1};\n"
            "Type_Info_Integer typeinfo_unsigned_short = {{ TYPE_TAG_INTEGER, sizeof(unsigned "
            "short), TYPEINFO_ALIGNOF(unsigned short) }, 0};\n"
            "Type_Info_Integer typeinfo_int = {{ TYPE_TAG_INTEGER, sizeof(int), "
            "TYPEINFO_ALIGNOF(int) }, 1};\n"
            "Type_Info_Integer typeinfo_unsigned_int = {{ TYPE_TAG_INTEGER, sizeof(unsigned int), "
            "TYPEINFO_ALIGNOF(unsigned int) }, 0};\n"
            "Type_Info_Integer typeinfo_long = {{ TYPE_TAG_INTEGER, sizeof(long), "
            "TYPEINFO_ALIGNOF(long) }, 1};\n"
            "Type_Info_Integer typeinfo_unsigned_long = {{ TYPE_TAG_INTEGER, sizeof(unsigned "
            "long), TYPEINFO_ALIGNOF(unsigned long) }, 0};\n"
            "Type_Info_Integer typeinfo_long_long = {{ TYPE_TAG_INTEGER, sizeof(long long), "
            "TYPEINFO_ALIGNOF(long long) }, 1};\n"
            "Type_Info_Integer typeinfo_unsigned_long_long = {{ TYPE_TAG_INTEGER, sizeof(unsigned "
            "long long), TYPEINFO_ALIGNOF(unsigned long long) }, 0};\n"
            "Type_Info_Float typeinfo_float = {{ TYPE_TAG_FLOAT, sizeof(float), "
            "TYPEINFO_ALIGNOF(float) }};\n"
            "Type_Info_Float typeinfo_double = {{ TYPE_TAG_FLOAT, sizeof(double), "
            "TYPEINFO_ALIGNOF(double) }};\n"
            "Type_Info_Float typeinfo_long_double = {{ TYPE_TAG_FLOAT, sizeof(long double), "
            "TYPEINFO_ALIGNOF(long double) }};\n\n");
}

static void emit_indentation(FILE* out, int indent) {
    for(int i = 0; i < indent; i++) fprintf(out, " ");
}

static void print_offset_error(const char* name, long long code) {
    fprintf(stderr, "Error computing offset of field '%s': ", name);
    switch(code) {
    case CXTypeLayoutError_Invalid:
        fprintf(stderr, "layout error: invalid cursor or not a field\n");
        break;
    case CXTypeLayoutError_Incomplete:
        fprintf(stderr, "layout error: field's type is incomplete\n");
        break;
    case CXTypeLayoutError_Dependent:
        fprintf(stderr, "layout error: field's type is dependent (e.g., template)\n");
        break;
    case CXTypeLayoutError_NotConstantSize:
        fprintf(stderr, "layout error: type has no constant size\n");
        break;
    case CXTypeLayoutError_InvalidFieldName:
        fprintf(stderr, "layout error: invalid field name in record\n");
        break;
    case CXTypeLayoutError_Undeduced:
        fprintf(stderr, "layout error: type is undeduced (template context)\n");
        break;
    default:
        fprintf(stderr, "unknown layout error (%lld)\n", code);
        break;
    }
}

static enum CXChildVisitResult check_typeinfo_attr(CXCursor c, CXCursor parent,
                                                   CXClientData client_data) {
    (void)parent;
    bool* found = (bool*)client_data;
    if(clang_getCursorKind(c) == CXCursor_AnnotateAttr) {
        CXString text = clang_getCursorDisplayName(c);
        const char* str = clang_getCString(text);
        if(str && strcmp(str, TYPE_INFO_ANNOTATION) == 0) {
            *found = true;
            clang_disposeString(text);
            return CXChildVisit_Break;
        }
        clang_disposeString(text);
    }
    return CXChildVisit_Recurse;
}

static bool has_typeinfo_annotation(CXCursor decl) {
    bool found = false;
    clang_visitChildren(decl, check_typeinfo_attr, &found);
    return found;
}

static enum CXChildVisitResult count_enum_values(CXCursor c, CXCursor parent, CXClientData data) {
    (void)c, (void)parent;
    assert(clang_getCursorKind(c) == CXCursor_EnumConstantDecl);
    (*(int*)data)++;
    return CXChildVisit_Continue;
}

static enum CXVisitorResult count_fields(CXCursor c, CXClientData data) {
    (void)c;
    (*(int*)data)++;
    return CXVisit_Continue;
}

static enum CXChildVisitResult collect_annotations(CXCursor c, CXCursor parent,
                                                   CXClientData client_data) {
    (void)parent;
    Annotations* out = (Annotations*)client_data;
    if(clang_getCursorKind(c) == CXCursor_AnnotateAttr) {
        CXString text = clang_getCursorDisplayName(c);
        const char* str = clang_getCString(text);
        if(str && strcmp(str, TYPE_INFO_ANNOTATION) != 0) {
            array_push(out, temp_strdup(str));
        }
        clang_disposeString(text);
    }
    return CXChildVisit_Continue;
}

static void emit_annotations_for_cursor(CXCursor c, FILE* out) {
    void* temp;
    defer_loop(temp = temp_checkpoint(), temp_rewind(temp)) {
        Annotations annotations = {.allocator = &temp_allocator};
        clang_visitChildren(c, collect_annotations, &annotations);
        fprintf(out, "(char*[]){ ");
        array_foreach(char*, it, &annotations) {
            fprintf(out, "\"%s\", ", *it);
        }
        fprintf(out, "NULL }");
    }
}

static void emit_typeinfo_for_type(Type_Info_Context* ctx, CXType type);
static enum CXChildVisitResult enum_value_visitor(CXCursor c, CXCursor parent, CXClientData data);
static enum CXVisitorResult member_visitor(CXCursor c, CXClientData data);

static void enqueue_type_if_needed(Type_Info_Context* ctx, CXType type) {
    type = clang_getCanonicalType(type);

    if(builtin_symbol(type.kind)) {
        return;
    }

    CXCursor decl = clang_getTypeDeclaration(type);
    if(clang_Cursor_isNull(decl) || clang_Cursor_isAnonymous(decl)) {
        return;
    }

    CXString spelling = clang_getCursorSpelling(decl);
    const char* name = clang_getCString(spelling);
    if(!name || !*name) {
        clang_disposeString(spelling);
        return;
    }

    Visited_Type_Entry* visited;
    hmap_get_cstr(ctx->visited_types, (char*)name, &visited);
    if(visited) {
        clang_disposeString(spelling);
        return;
    }

    array_push(ctx->pending_types, type);  // Add to queue
    clang_disposeString(spelling);
}

static void emit_typeinfo_for_type(Type_Info_Context* ctx, CXType type) {
    FILE* out = ctx->source;

    long long num_elems = clang_getNumElements(type);
    if(num_elems >= 0) {  // It's an array
        long long const_size = clang_getArraySize(type);

        long long count = (const_size >= 0 ? const_size : num_elems);
        long long array_size = clang_Type_getSizeOf(type);
        long long array_align = clang_Type_getAlignOf(type);
        fprintf(out, "(Type_Info*)&(Type_Info_Array){{TYPE_TAG_ARRAY, %lld, %lld}, %lld, ",
                array_size, array_align, count);

        CXType elem = clang_getArrayElementType(type);
        emit_typeinfo_for_type(ctx, clang_getCanonicalType(elem));

        fprintf(out, " }");
    } else if(type.kind == CXType_IncompleteArray) {  // Flexible array member
        CXType elem = clang_getArrayElementType(type);
        long long elem_align = clang_Type_getAlignOf(clang_getCanonicalType(elem));
        if(elem_align < 0) elem_align = 0;
        fprintf(out, "(Type_Info*)&(Type_Info_Array){{TYPE_TAG_ARRAY, 0, %lld}, 0, ", elem_align);
        emit_typeinfo_for_type(ctx, clang_getCanonicalType(elem));
        fprintf(out, " }");
    } else if(type.kind == CXType_Pointer) {
        CXType pointee = clang_getPointeeType(type);
        long long ptr_align = clang_Type_getAlignOf(type);
        fprintf(out, "(Type_Info*)&(Type_Info_Pointer){{TYPE_TAG_POINTER, sizeof(void*), %lld}, ",
                ptr_align);

        if(pointee.kind == CXType_FunctionProto || pointee.kind == CXType_FunctionNoProto) {
            fprintf(out, "NULL");
        } else {
            emit_typeinfo_for_type(ctx, clang_getCanonicalType(pointee));
        }
        fprintf(out, ", ");
        emit_qualifier_flags(out, pointee);
        fprintf(out, " }");
    } else if(type.kind == CXType_Enum) {
        CXCursor decl = clang_getTypeDeclaration(type);

        if(clang_Cursor_isAnonymous(decl)) {  // Anonymous enum, emit inline
            // Count enum values
            int value_count = 0;
            clang_visitChildren(decl, count_enum_values, &value_count);

            long long esize = clang_Type_getSizeOf(type);
            long long ealign = clang_Type_getAlignOf(type);
            fprintf(out, "(Type_Info*)&(Type_Info_Enum){{TYPE_TAG_ENUM, %lld, %lld}, ", esize,
                    ealign);

            emit_annotations_for_cursor(decl, out);

            fprintf(out, ", \"\", (Type_Info_Enum_Value[]){\n");
            ctx_indent(ctx) {
                clang_visitChildren(decl, enum_value_visitor, ctx);
            }
            emit_indentation(out, ctx->indent);
            fprintf(out, "}, %d }", value_count);
        } else {  // Named enum - reference by name
            enqueue_type_if_needed(ctx, type);
            CXString en = clang_getCursorSpelling(decl);
            fprintf(out, "(Type_Info*)&typeinfo_%s", clang_getCString(en));
            clang_disposeString(en);
        }
    } else if(type.kind == CXType_Record) {
        CXCursor decl = clang_getTypeDeclaration(type);
        enum CXCursorKind kind = clang_getCursorKind(decl);

        if(clang_Cursor_isAnonymous(decl)) {  // Anonymous struct/union, emit inline
            int field_count = 0;
            clang_Type_visitFields(type, count_fields, &field_count);

            const char* tag = (kind == CXCursor_UnionDecl) ? "TYPE_TAG_UNION" : "TYPE_TAG_STRUCT";
            const char* ti = (kind == CXCursor_UnionDecl) ? "Type_Info_Union" : "Type_Info_Struct";
            long long ssize = clang_Type_getSizeOf(type);
            long long salign = clang_Type_getAlignOf(type);
            fprintf(out, "(Type_Info*)&(%s){{%s, %lld, %lld}, ", ti, tag, ssize, salign);

            emit_annotations_for_cursor(decl, out);

            fprintf(out, ", \"\", (Type_Info_Member[]){\n");
            ctx_indent(ctx) {
                clang_Type_visitFields(type, member_visitor, ctx);
            }
            emit_indentation(out, ctx->indent);
            fprintf(out, "}, %d }", field_count);
        } else {  // Named struct/union - reference by name
            enqueue_type_if_needed(ctx, type);
            CXString sn = clang_getCursorSpelling(decl);
            fprintf(out, "(Type_Info*)&typeinfo_%s", clang_getCString(sn));
            clang_disposeString(sn);
        }
    } else {  // Otherwise, base type
        const char* base_sym = builtin_symbol(type.kind);
        if(base_sym) {
            fprintf(out, "(Type_Info*)&typeinfo_%s", base_sym);
        } else {
            enqueue_type_if_needed(ctx, type);
            CXCursor decl = clang_getTypeDeclaration(type);
            CXString sn = clang_getCursorSpelling(decl);
            fprintf(out, "(Type_Info*)&typeinfo_%s", clang_getCString(sn));
            clang_disposeString(sn);
        }
    }
}

static enum CXVisitorResult member_visitor(CXCursor c, CXClientData data) {
    Type_Info_Context* ctx = data;
    FILE* out = ctx->source;

    assert(clang_getCursorKind(c) == CXCursor_FieldDecl);

    CXString name = clang_getCursorSpelling(c);
    const char* field_name = clang_getCString(name);
    // Unnamed anonymous inner structs return a field name that contists of the parent struct name
    // plus the declaration path inside parethesis. If this is the case, we forcefully set the name
    // to the empty string.
    if(strchr(field_name, '(')) field_name = "";

    // To extract qualifiers before canonicalization possibly strips them.
    CXType declared_type = clang_getCursorType(c);
    CXType type = clang_getCanonicalType(declared_type);

    long long offset_bits = clang_Cursor_getOffsetOfField(c);
    if(offset_bits < 0) {
        print_offset_error(field_name, offset_bits);
        clang_disposeString(name);
        return CXVisit_Break;
    }

    long long offset_bytes = offset_bits / 8;

    emit_indentation(out, ctx->indent);
    fprintf(out, "{ ");
    emit_annotations_for_cursor(c, out);
    fprintf(out, ", \"%s\", %lld, ", field_name, offset_bytes);
    emit_typeinfo_for_type(ctx, type);
    fprintf(out, ", ");
    emit_qualifier_flags(out, declared_type);
    fprintf(out, " },\n");

    clang_disposeString(name);

    return CXVisit_Continue;
}

static enum CXChildVisitResult enum_value_visitor(CXCursor c, CXCursor parent, CXClientData data) {
    (void)parent;
    Type_Info_Context* ctx = data;
    FILE* out = ctx->source;
    if(clang_getCursorKind(c) == CXCursor_EnumConstantDecl) {
        CXString name = clang_getCursorSpelling(c);
        long long value = clang_getEnumConstantDeclValue(c);

        emit_indentation(out, ctx->indent);
        fprintf(out, "{ ");
        emit_annotations_for_cursor(c, out);
        fprintf(out, ", \"%s\", %lld },\n", clang_getCString(name), value);

        clang_disposeString(name);
    }
    return CXChildVisit_Continue;
}

static void process_queued_type(Type_Info_Context* ctx, CXType type) {
    type = clang_getCanonicalType(type);
    CXCursor c = clang_getTypeDeclaration(type);

    if(clang_Cursor_isNull(c) || clang_Cursor_isAnonymous(c) ||
       clang_Type_getSizeOf(type) == CXTypeLayoutError_Incomplete) {
        return;
    }

    enum CXCursorKind kind = clang_getCursorKind(c);
    if(kind < CXCursor_FirstDecl || kind > CXCursor_LastDecl) {
        return;
    }

    long long size = clang_Type_getSizeOf(type);
    long long align = clang_Type_getAlignOf(type);
    assert(size >= 0);
    assert(align >= 0);

    CXString spelling = clang_getCursorSpelling(c);
    const char* name = clang_getCString(spelling);

    Visited_Type_Entry* visited;
    hmap_get_cstr(ctx->visited_types, (char*)name, &visited);
    if(visited) {
        clang_disposeString(spelling);
        return;
    }
    hmap_put_cstr(ctx->visited_types, temp_strdup(name), true);

    FILE* header = ctx->header;
    FILE* source = ctx->source;

    CXFile file;
    unsigned line, column, offset;
    clang_getExpansionLocation(clang_getCursorLocation(c), &file, &line, &column, &offset);
    CXString filename_str = clang_getFileName(file);
    const char* filename = clang_getCString(filename_str);

    switch(kind) {
    case CXCursor_StructDecl:
    case CXCursor_UnionDecl: {
        if(kind == CXCursor_StructDecl) {
            fprintf(header, "extern Type_Info_Struct typeinfo_%s; // %s:%u:%u\n", name, filename,
                    line, column);
            fprintf(source, "// struct %s\n", name);
        } else {
            fprintf(header, "extern Type_Info_Union typeinfo_%s; // %s:%u:%u\n", name, filename,
                    line, column);
            fprintf(source, "// union %s\n", name);
        }

        fprintf(source,
                "// %s:%u:%u\n"
                "static Type_Info_Member members_%s[] = {\n",
                filename, line, column, name);

        ctx_indent(ctx) {
            clang_Type_visitFields(type, member_visitor, ctx);
        }

        fprintf(source, "};\n");

        if(kind == CXCursor_StructDecl) {
            fprintf(source,
                    "Type_Info_Struct typeinfo_%s = {\n"
                    "  { TYPE_TAG_STRUCT, %lld, %lld },\n",
                    name, size, align);
        } else {
            fprintf(source,
                    "Type_Info_Union typeinfo_%s = {\n"
                    "  { TYPE_TAG_UNION, %lld, %lld },\n",
                    name, size, align);
        }

        emit_indentation(source, INDENT);
        emit_annotations_for_cursor(c, source);
        fprintf(source, ",\n");

        fprintf(source,
                "  \"%s\",\n"
                "  members_%s,\n"
                "  sizeof(members_%s)/sizeof(*members_%s)\n"
                "};\n\n",
                name, name, name, name);
    } break;

    case CXCursor_EnumDecl: {
        fprintf(header, "extern Type_Info_Enum typeinfo_%s; // %s:%u:%u \n", name, filename, line,
                column);

        fprintf(source,
                "// enum %s\n"
                "// %s:%u:%u\n"
                "static Type_Info_Enum_Value values_%s[] = {\n",
                name, filename, line, column, name);

        ctx_indent(ctx) {
            clang_visitChildren(c, enum_value_visitor, ctx);
        }

        fprintf(source, "};\n");

        fprintf(source,
                "Type_Info_Enum typeinfo_%s = {\n"
                "  { TYPE_TAG_ENUM, %lld, %lld },\n",
                name, size, align);

        emit_indentation(source, INDENT);
        emit_annotations_for_cursor(c, source);
        fprintf(source, ",\n");

        fprintf(source,
                "  \"%s\",\n"
                "  values_%s,\n"
                "  sizeof(values_%s)/sizeof(*values_%s)\n"
                "};\n\n",
                name, name, name, name);
    } break;

    default:
        break;
    }

    clang_disposeString(spelling);
    clang_disposeString(filename_str);
}

static enum CXChildVisitResult queue_types(CXCursor c, CXCursor parent, CXClientData data) {
    (void)parent;
    Type_Info_Context* ctx = data;

    if(!has_typeinfo_annotation(c) || !clang_isCursorDefinition(c) || clang_Cursor_isAnonymous(c)) {
        return CXChildVisit_Recurse;
    }

    enum CXCursorKind kind = clang_getCursorKind(c);
    if(kind < CXCursor_FirstDecl || kind > CXCursor_LastDecl) {
        return CXChildVisit_Recurse;
    }

    CXType type = clang_getCursorType(c);
    if(clang_Type_getSizeOf(type) == CXTypeLayoutError_Incomplete) {
        return CXChildVisit_Continue;
    }

    assert(clang_Type_getSizeOf(type) >= 0);
    array_push(ctx->pending_types, type);

    return CXChildVisit_Recurse;
}

static void print_usage(const char* program_name, FILE* stream) {
    fprintf(stream, "USAGE: %s [OPTIONS] [FILE...]\n", program_name);
    fprintf(stream, "OPTIONS\n");
    fprintf(stream, "  -o <out_name>       base name of generated file (REQUIRED)\n");
    fprintf(stream, "  -I<dir>             add include path (forwarded to clang)\n");
    fprintf(stream, "  -std=<std>          set language standard (forwarded to clang)\n");
    fprintf(stream,
            "  -no-builtin-types   do not emit builtin type info declarations/definitions\n");
    fprintf(
        stream,
        "  -no-absolute-sizes  emit sizeof/offsetof/TYPEINFO_ALIGNOF instead of absolue values\n");
    fprintf(stream, "  -R                  recursively walk directories\n");
    fprintf(stream, "  -h                  prints this help message and exit\n");
}

static bool process_file(const char* file_path, CXIndex index, Type_Info_Context* ctx) {
    CXTranslationUnit unit =
        clang_parseTranslationUnit(index, file_path, (const char**)opts.forwarded.items,
                                   (int)opts.forwarded.size, NULL, 0, CXTranslationUnit_None);

    bool has_error = false;
    unsigned num_diags = clang_getNumDiagnostics(unit);

    for(unsigned i = 0; i < num_diags; ++i) {
        CXDiagnostic diag = clang_getDiagnostic(unit, i);
        enum CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diag);

        const char* type_str;
        switch(severity) {
        case CXDiagnostic_Ignored:
            type_str = "ignored";
            break;
        case CXDiagnostic_Note:
            type_str = "note";
            break;
        case CXDiagnostic_Warning:
            type_str = "warning";
            break;
        case CXDiagnostic_Error:
            type_str = "error";
            has_error = true;
            break;
        case CXDiagnostic_Fatal:
            type_str = "fatal error";
            has_error = true;
            break;
        default:
            UNREACHABLE();
        }

        CXString diag_str = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
        fprintf(stderr, "%s: %s\n", type_str, clang_getCString(diag_str));
        clang_disposeString(diag_str);
        clang_disposeDiagnostic(diag);
    }

    if(has_error) {
        clang_disposeTranslationUnit(unit);
        return false;
    }

    if(!unit) {
        fprintf(stderr, "Error parsing %s\n", file_path);
        return false;
    }

    CXCursor root = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(root, queue_types, ctx);

    size_t processed = 0;
    while(processed < ctx->pending_types->size) {
        CXType type = ctx->pending_types->items[processed];
        process_queued_type(ctx, type);
        processed++;
    }

    ctx->pending_types->size = 0;
    clang_disposeTranslationUnit(unit);

    return true;
}

static bool process_path(const char* path, CXIndex index, Type_Info_Context* ctx);

static bool process_directory(const char* dir_path, CXIndex index, Type_Info_Context* ctx) {
    Paths paths = {0};
    if(!read_dir(dir_path, &paths)) {
        fprintf(stderr, "Error reading directory %s\n", dir_path);
        return false;
    }

    bool ok = true;
    array_foreach(char*, it, &paths) {
        void* temp;
        defer_loop(temp = temp_checkpoint(), temp_rewind(temp)) {
            const char* entry = *it;
            const char* full;
            size_t len = strlen(dir_path);
            if(len > 0 && dir_path[len - 1] == '/') {
                full = temp_sprintf("%s%s", dir_path, entry);
            } else {
                full = temp_sprintf("%s/%s", dir_path, entry);
            }
            ok &= process_path(full, index, ctx);
        }
    }

    free_paths(&paths);
    return ok;
}

static bool process_path(const char* path, CXIndex index, Type_Info_Context* ctx) {
    FileType type = get_file_type(path);
    switch(type) {
    case FILE_SYMLINK:
    case FILE_REGULAR:
        return process_file(path, index, ctx);
    case FILE_DIR:
        if(!opts.recursive) {
            fprintf(stderr, "Skipping directory '%s' (use -R to recurse)\n", path);
            return true;
        }
        return process_directory(path, index, ctx);
    case FILE_OTHER:
        fprintf(stderr, "Skipping file '%s': unknown file type\n", path);
        return true;
    case FILE_ERR:
        return false;
    }
    UNREACHABLE();
}

static void parse_arguments(int argc, char** argv) {
    char* program_name = shift(argc, argv);

    int npos = 0;
    for(int i = 0; i < argc; i++) {
        if(strcmp("--", argv[i]) == 0) {  // All arguments after `--` are positional
            for(i++; i < argc; i++) argv[npos++] = argv[i];
            break;
        }

        if(strlen(argv[i]) <= 1 || argv[i][0] != '-') {
            argv[npos++] = argv[i];
            continue;
        }

        if(strcmp("-h", argv[i]) == 0) {
            print_usage(program_name, stdout);
            exit(0);
        } else if(strcmp("-R", argv[i]) == 0) {
            opts.recursive = true;
        } else if(strcmp("-no-builtin-types", argv[i]) == 0) {
            opts.no_builtin_types = true;
        } else if(strcmp("-o", argv[i]) == 0) {
            if(i + 1 >= argc) {
                fprintf(stderr, "no argument for option `-o`\n");
                print_usage(program_name, stderr);
                exit(1);
            }
            opts.out = argv[++i];
        } else if(ss_starts_with(SS(argv[i]), SS("-I"))) {
            if(argv[i][2] != '\0') {
                array_push(&opts.forwarded, argv[i]);
            } else {
                if(i + 1 >= argc) {
                    fprintf(stderr, "no argument for option `-I`\n");
                    print_usage(program_name, stderr);
                    exit(1);
                }
                array_push(&opts.forwarded, "-I");
                array_push(&opts.forwarded, argv[++i]);
            }
        } else if(ss_starts_with(SS(argv[i]), SS("-std="))) {
            array_push(&opts.forwarded, argv[i]);
        }
    }

    if(!opts.out) {
        fprintf(stderr, "no output name (`-o`) specified\n");
        print_usage(program_name, stderr);
        exit(1);
    }

    opts.files = argv;
    opts.count = npos;
}
int main(int argc, char** argv) {
    parse_arguments(argc, argv);
    array_push(&opts.forwarded, RUNNING_METAPROGRAM);
    array_push(&opts.forwarded, "-x");
    array_push(&opts.forwarded, "c");

    char* header_name = temp_sprintf("%s.h", opts.out);
    char* source_name = temp_sprintf("%s.c", opts.out);

    StringSlice header_basename = SS(header_name);
    header_basename = ss_rsplit_once(&header_basename, '/');

    FILE* header = fopen(header_name, "w");
    FILE* source = fopen(source_name, "w");
    if(!header || !source) {
        fprintf(stderr, "error opening output files: %s\n", strerror(errno));
        if(header) fclose(header);
        if(source) fclose(source);
        return 1;
    }

    StringBuffer include_guard = {.allocator = &temp_allocator.base};
    sb_append(&include_guard, header_basename.data, header_basename.size);
    sb_replace(&include_guard, 0, ". ", '_');
    for(size_t i = 0; i < include_guard.size; i++) {
        include_guard.items[i] = toupper(include_guard.items[i]);
    }

    // Builtin typeinfo declarations
    fprintf(header,
            "#ifndef %.*s_\n"
            "#define %.*s_\n\n"
            "#include \"typeinfo.h\"\n\n",
            SB_Arg(include_guard), SB_Arg(include_guard));
    if(!opts.no_builtin_types) emit_builtin_decls(header);

    // Builtin typeinfo definitions
    fprintf(source, "#include \"typeinfo.h\"\n");
    fprintf(source, "#include \"" SS_Fmt "\"\n\n", SS_Arg(header_basename));
    if(!opts.no_builtin_types) emit_builtin_defs(source);

    Visited_Types visited = {0};
    Type_Queue pending = {0};
    Type_Info_Context ctx = {
        .header = header,
        .source = source,
        .indent = 0,
        .visited_types = &visited,
        .pending_types = &pending,
    };

    int result = 0;
    CXIndex index;
    defer_loop(index = clang_createIndex(0, 0), clang_disposeIndex(index)) {
        for(int i = 0; i < opts.count; i++) {
            if(!process_path(opts.files[i], index, &ctx)) {
                result = 1;
            }
        }
    }

    fprintf(header, "\n#endif // %.*s_\n", SB_Arg(include_guard));
    fclose(header);
    fclose(source);

    printf("Generated: %s and %s\n", header_name, source_name);
    printf("Generated: %zu type infos\n", visited.size);

    hmap_free(&visited);
    array_free(&pending);

    return result;
}
