.PHONY: all clean

ifeq ($(OS),Windows_NT)
    DLLEXT := .dll
else
    DLLEXT := .so
endif

CC = gcc
CC99 = c99
CFLAGS = -c -std=c89 -Wall -pedantic -Werror
LDFLAGS = -lSDL2 -lSDL2main -lGL
LIBNAME = libblaze$(DLLEXT)
LIBNAME_TEST = libblaze-test$(DLLEXT)

TESTS = $(wildcard test/test_*.c)
TEST_NAMES = $(patsubst test/%.c, %.out, $(TESTS))

SOIL_SOURCES = $(wildcard deps/SOIL/*.c)
SOIL_OBJS = $(patsubst deps/SOIL/%.c, deps/SOIL/%.o, $(SOIL_SOURCES))

all: $(LIBNAME) $(TEST_NAMES)

$(LIBNAME): blaze.c blaze.h $(SOIL_OBJS)
	$(info >>> Compiling a shared library $@)
	$(CC) $(CFLAGS) -fPIC blaze.h blaze.c
	$(CC) -shared -o $@ blaze.o $(SOIL_OBJS)

$(LIBNAME_TEST): blaze.c blaze.h $(SOIL_OBJS) glad.o
	$(info >>> Compiling a shared library with TEST flag $@)
	$(CC) $(CFLAGS) -fPIC -g blaze.h blaze.c -D TEST -ftest-coverage -fprofile-arcs
	$(CC) -shared -o $@ blaze.o $(SOIL_OBJS) glad.o -lgcov

common.o: test/common.h test/common.c
	$(CC99) -c -g test/common.h test/common.c

tap.o: deps/tap.c/tap.c
	$(CC) -g -c $< -o $@

test_%.o: test/test_%.c
	$(info >>> Compiling $@)
	$(CC99) -g -c $< -o $@

test_%.out: test_%.o $(LIBNAME_TEST) tap.o common.o
	$(info >>> Linking $@)
	$(CC99) $< $(LDFLAGS) -L. -l:$(LIBNAME_TEST) -l:tap.o -l:common.o -o $@

deps/SOIL/%.o: deps/SOIL/%.c
	$(info >>> Compiling $@)
	$(CC) -fPIC -c $< -o $@ -I "./deps/"

glad.o: glad/src/glad.c
	$(CC) -std=c89 -fPIC -c $< -o $@ -I "./glad/include/" -g

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf *$(DLLEXT)
	rm -rf *.out
	rm -rf deps/SOIL/*.o
	rm -rf deps/SOIL/*.a
	rm -rf deps/SOIL/*$(DLLEXT)
	rm -rf deps/SOIL/*.out
