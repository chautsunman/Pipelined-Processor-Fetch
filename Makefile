
all: fetch

CC=gcc
CLIBS=-lc
CFLAGS=-g -Werror-implicit-function-declaration -pedantic -std=c99

FETCHOBJS=fetchStage.o printInternalReg.o


fetch: $(FETCHOBJS)
	$(CC) -g -o fetch $(FETCHOBJS)

fetchStage.o: fetchStage.c printInternalReg.h
printInternalReg.o: printInternalReg.c printInternalReg.h


clean:
	rm -f *.o
	rm -f fetch

