.PHONY: all clean

CC = gcc
CFLAGS = -c -Wall
LDFLAGS = -lSDL2 -lSDL2main -lGL
LIBNAME = libblaze.so

TESTS = $(wildcard test/test_*.c)

all: $(LIBNAME) $(patsubst %.c, %, $(TESTS))
	$(info Tests found: $(TESTS))

blaze.o: blaze.c blaze.h
	$(CC) $(CFLAGS) -fPIC blaze.h blaze.c

$(LIBNAME): blaze.o
	$(CC) -shared -o $(LIBNAME) blaze.o

test_%.o: test/test_%.c
	$(CC) $(CFLAGS) $< -o $@

test_%: test/test_%.o
	$(CC) $< $(LDFLAGS) -l:$(LIBNAME) -o $@

test_%: test_%.o $(LIBNAME)
	$(CC)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so
	rm -rf test/*.o
	rm -rf test/*.a
	rm -rf test/*.so
