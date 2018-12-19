.PHONY: all clean

CC = gcc
CC99 = c99
CFLAGS = -c -std=c89 -Wall -pedantic -Werror -g
LDFLAGS = -lSDL2 -lSDL2main -lGL
LIBNAME = libblaze.so

TESTS = $(wildcard test/test_*.c)
TEST_NAMES = $(patsubst test/%.c, %.test, $(TESTS))

all: $(LIBNAME) $(TEST_NAMES)

blaze.o: blaze.c blaze.h
	$(info >>> Compiling $@)
	$(CC) $(CFLAGS) -fPIC blaze.h blaze.c

$(LIBNAME): blaze.o
	$(info >>> Making a shared library $@)
	$(CC) -shared -o $(LIBNAME) blaze.o

tap.o: deps/tap.c/tap.c
	$(CC99) -c $< -o $@

test_%.o: test/test_%.c
	$(info >>> Compiling $@)
	$(CC99) -c $< -o $@

test_%.test: test_%.o $(LIBNAME) tap.o
	$(info >>> Linking $@)
	$(CC99) $< $(LDFLAGS) -L. -l:$(LIBNAME) -l:tap.o -o $@

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so
	rm -rf *.test
