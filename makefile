CC = clang
CFLAGS = -Wno-unused-command-line-argument -Wall -Wextra --std=gnu17

OPT_LEVEL = -O2

all: bin/main.o bin/fileformat.o bin/lbvm

clean:
	rm -rf bin/*

bin/fileformat.o: src/fileformat.c src/fileformat.h src/common.h src/debug_utils.h src/values.h src/machine.h
	$(CC) $(CFLAGS) $(OPT_LEVEL) -c src/fileformat.c -o bin/fileformat.o

bin/main.o: src/main.c src/common.h src/debug_utils.h src/values.h src/machine.h
	$(CC) $(CFLAGS) $(OPT_LEVEL) -c src/main.c -o bin/main.o

bin/lbvm: bin/fileformat.o bin/main.o
	$(CC) $(CFLAGS) $(OPT_LEVEL) bin/*.o -o bin/lbvm
