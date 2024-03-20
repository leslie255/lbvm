CC = clang
CFLAGS = -Wno-unused-command-line-argument -Wall -Wconversion --std=gnu2x

OPT_LEVEL = -O2

all: bin/main.o bin/lbvm

clean:
	rm -rf bin/*

bin/main.o: src/main.c src/common.h src/values.h src/machine.h src/debug_utils.h
	$(CC) $(CFLAGS) $(OPT_LEVEL) -c src/main.c -o bin/main.o

bin/lbvm: bin/main.o
	$(CC) $(CFLAGS) $(OPT_LEVEL) bin/*.o -o bin/lbvm
