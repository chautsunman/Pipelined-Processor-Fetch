#This is a hack to pass arguments to the run command and probably only 
#works with gnu make. 
#You can run the fetch program by giving doing the following
# make run arg1 arg2
# where arg1 and arg2 are the arguments to the fetch program
#
ifeq (run,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "run"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
endif
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

run: fetch  
	./fetch $(RUN_ARGS)