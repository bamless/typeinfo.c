#ifndef PRINT_TYPES_TYPEINFO_H_
#define PRINT_TYPES_TYPEINFO_H_

#include "typeinfo.h"

extern Type_Info_Void typeinfo_void;
extern Type_Info_Integer typeinfo_bool;
extern Type_Info_Integer typeinfo_char;
extern Type_Info_Integer typeinfo_signed_char;
extern Type_Info_Integer typeinfo_unsigned_char;
extern Type_Info_Integer typeinfo_short;
extern Type_Info_Integer typeinfo_unsigned_short;
extern Type_Info_Integer typeinfo_int;
extern Type_Info_Integer typeinfo_unsigned_int;
extern Type_Info_Integer typeinfo_long;
extern Type_Info_Integer typeinfo_unsigned_long;
extern Type_Info_Integer typeinfo_long_long;
extern Type_Info_Integer typeinfo_unsigned_long_long;
extern Type_Info_Float typeinfo_float;
extern Type_Info_Float typeinfo_double;
extern Type_Info_Float typeinfo_long_double;

extern Type_Info_Struct typeinfo_Foo; // /home/fabrizio/Workspace/c/typeinfo.c/examples/print_types.h:24:9
extern Type_Info_Enum typeinfo_Color; // /home/fabrizio/Workspace/c/typeinfo.c/examples/print_types.h:30:9 
extern Type_Info_Union typeinfo_TestUnion; // /home/fabrizio/Workspace/c/typeinfo.c/examples/print_types.h:39:9
extern Type_Info_Struct typeinfo_TestAnonymousEnum; // /home/fabrizio/Workspace/c/typeinfo.c/examples/print_types.h:49:9
extern Type_Info_Struct typeinfo_TestUnnamedAnonymous; // /home/fabrizio/Workspace/c/typeinfo.c/examples/print_types.h:54:9
extern Type_Info_Struct typeinfo_TestQualifiers; // /home/fabrizio/Workspace/c/typeinfo.c/examples/print_types.h:65:9
extern Type_Info_Struct typeinfo_Bar; // /home/fabrizio/Workspace/c/typeinfo.c/examples/print_types.h:13:9
extern Type_Info_Struct typeinfo_Baz; // /home/fabrizio/Workspace/c/typeinfo.c/examples/print_types.h:8:9

#endif // PRINT_TYPES_TYPEINFO_H_
