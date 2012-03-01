PRGNAME = refcount
OBJ = refcount.o main.o
CFLAGS = -std=c99 -pedantic -Wall -W -g -O2 -pthread -ansi \
		-D _POSIX_C_SOURCE=200809L

all: refcount

refcount: $(OBJ)
	$(CC) -o $(PRGNAME) $(CFLAGS) $(OBJ)

main.o: main.c refcount.h

refcount.o: refcount.c refcount.h

.PHONY: all clean

clean:
	rm -rf $(PRGNAME) *.o

dep:
	$(CC) -MM *.c