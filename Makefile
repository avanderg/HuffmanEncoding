# Aaron VanderGraaff
# CPE357 Assgn3 Makefile

CC = gcc
CFLAGS =  -Wall -g -D_GNU_SOURCE -pedantic

all: hencode hdecode huffman_funs.o
hencode: hencode.o huffman_funs.o
	$(CC) $(CFLAGS) -O2 -o hencode hencode.o hencode.h huffman_funs.o huffman_funs.h
hencode.o: hencode.c hencode.h huffman_funs.h
	$(CC) $(CFLAGS) -c -O2 hencode.c
hdecode: hdecode.o huffman_funs.o
	$(CC) $(CFLAGS) -O2 -o hdecode hdecode.o hdecode.h huffman_funs.o huffman_funs.h	
hdecode.o: hdecode.c hdecode.h huffman_funs.h
	$(CC) $(CFLAGS) -c -O2 hdecode.c
huffman_funs.o: huffman_funs.c huffman_funs.h
	$(CC) $(CFLAGS) -c -O2 huffman_funs.c
clean:
	rm  *.o

