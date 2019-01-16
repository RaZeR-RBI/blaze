.PHONY: all clean

INCLUDES = -I "./glad/include/"
CC = gcc
CFLAGS = -c -std=c99 -Wall -pedantic -Werror $(INCLUDES)
OPTIMIZE = -O2
LDFLAGS_TEST = -lSDL2main -lSDL2
DEBUG = -ggdb

ifeq ($(OS),Windows_NT)
    DLLEXT := .dll
    LDFLAGS_LIB = -lopengl32
else
    DLLEXT := .so
	LDFLAGS_LIB = -lGL
	# Uncomment to use ASan
    # DEBUG += -fsanitize=address
endif

LIBNAME = libblaze$(DLLEXT)
LIBNAME_TEST = libblaze-test$(DLLEXT)

TESTS = $(wildcard test/test_*.c)
TEST_NAMES = $(patsubst test/%.c, %.out, $(TESTS))

SOIL_SOURCES = $(wildcard deps/SOIL/*.c)
SOIL_OBJS = $(patsubst deps/SOIL/%.c, deps/SOIL/%.o, $(SOIL_SOURCES))

all: $(LIBNAME) $(TEST_NAMES)

$(LIBNAME): blaze.c blaze.h $(SOIL_OBJS) glad.o
	$(info >>> Compiling a shared library $@)
	$(CC) $(CFLAGS) $(OPTIMIZE) -fPIC blaze.h blaze.c
	$(CC) -shared -o $@ blaze.o $(SOIL_OBJS) glad.o $(LDFLAGS_LIB)

$(LIBNAME_TEST): blaze.c blaze.h $(SOIL_OBJS) glad.o
	$(info >>> Compiling a shared library with TEST flag $@)
	$(CC) $(CFLAGS) -fPIC $(DEBUG) blaze.h blaze.c -D TEST -ftest-coverage -fprofile-arcs
	$(CC) -shared -o $@ blaze.o $(SOIL_OBJS) glad.o -lgcov $(LDFLAGS_LIB)

common.o: test/common.h test/common.c
	$(CC) -c $(DEBUG) test/common.h test/common.c $(INCLUDES)

tap.o: deps/tap.c/tap.c
	$(CC) $(DEBUG) -c $< -o $@

test_%.o: test/test_%.c
	$(info >>> Compiling $@)
	$(CC) $(DEBUG) -c $< -o $@

test_%.out: test_%.o $(LIBNAME_TEST) tap.o common.o
	$(info >>> Linking $@)
	$(CC) $< $(LDFLAGS_TEST) $(DEBUG) -L. -l:$(LIBNAME_TEST) -l:tap.o -l:common.o

deps/SOIL/%.o: deps/SOIL/%.c glad.o
	$(info >>> Compiling $@)
	$(CC) $(OPTIMIZE) -fPIC -c $< -o $@ -I "./deps/"

glad.o: glad/src/glad.c
	$(CC) $(OPTIMIZE) -fPIC -c $< -o $@ $(INCLUDES)

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
