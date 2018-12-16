CC=gcc
CFLAGS="-c"
OUTPUTFILE=libblaze.a

all: libblaze.a

blaze.o: blaze.c blaze.h
	$(CC) $(CFLAGS) blaze.h blaze.c

libblaze.a: blaze.o
	ar rcs $(OUTPUTFILE) blaze.o
