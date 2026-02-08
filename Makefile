CC=gcc
CFLAGS=-Wall -Wextra -std=c99
CLANG_INCLUDE=$(shell clang -print-resource-dir)/include

typeinfo_metaprogram: typeinfo_metaprogram.c extlib.h
	$(CC) $(CFLAGS) $< -o $@ -lclang

examples/print_types: examples/print_types.c examples/print_types_typeinfo.c
	$(CC) $(CFLAGS) -Iinclude -Iexamples $^ -o $@

examples/print_types_typeinfo.c: examples/print_types.h include/typeinfo.h typeinfo_metaprogram
	./typeinfo_metaprogram -Iinclude -I$(CLANG_INCLUDE) $< -o $(basename $@)

test/test: test/test.c test/test_types_typeinfo.c
	$(CC) $(CFLAGS) -Iinclude -Itest $^ -o $@

test/test_types_typeinfo.c: test/test_types.h include/typeinfo.h typeinfo_metaprogram
	./typeinfo_metaprogram -Iinclude -I$(CLANG_INCLUDE) $< -o $(basename $@)

.PHONY: test
test: test/test
	./test/test

.PHONY: clean
clean:
	rm -f examples/print_types examples/print_types_typeinfo.{c,h} test/test test/test_types_typeinfo.{c,h} typeinfo_metaprogram
