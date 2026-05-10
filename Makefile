CFLAGS        ?= -Wall -Wextra -std=c99
CFLAGS        += -MMD -MP
CLANG_INCLUDE  = $(shell clang -print-resource-dir)/include

.PHONY: all
all: examples/print_types

typeinfo_metaprogram: typeinfo_metaprogram.c
	$(CC) $(CFLAGS) $< -o $@ -lclang

# --------------------------------------------------------------------------------
# Examples
# --------------------------------------------------------------------------------

# Run the metaprogram
examples/print_types_typeinfo.c examples/print_types_typeinfo.h &: examples/print_types.h typeinfo_metaprogram
	./typeinfo_metaprogram -Iinclude -I$(CLANG_INCLUDE) $< -o $(basename $@)

examples/print_types: examples/print_types.c examples/print_types_typeinfo.c examples/print_types_typeinfo.h
	$(CC) $(CFLAGS) -Iinclude -Iexamples $(filter %.c,$^) -o $@

# --------------------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------------------

# Run the metaprogram
test/test_types_typeinfo.c test/test_types_typeinfo.h &: test/test_types.h typeinfo_metaprogram
	./typeinfo_metaprogram -Iinclude -I$(CLANG_INCLUDE) $< -o $(basename $@)

test/test: test/test.c test/test_types_typeinfo.c test/test_types_typeinfo.h
	$(CC) $(CFLAGS) -Iinclude -Itest $(filter %.c,$^) -o $@ -Wno-attributes -Wno-pragmas -Wno-unused-function

.PHONY: test
test: test/test
	./test/test

# --------------------------------------------------------------------------------
# Clean
# --------------------------------------------------------------------------------

.PHONY: clean
clean:
	rm -f examples/print_types examples/print_types.d examples/print_types_typeinfo.{c,h} \
		test/test test/test.d test/test_types_typeinfo.{c,h}                              \
		typeinfo_metaprogram typeinfo_metaprogram.d

-include examples/print_types.d
-include test/test.d
