CC = gcc
CFLAGS = -c -ggdb3 -Wall -Wextra -pthread
LFLAGS = -ggdb3 -Wall -Wextra -pthread

.PHONY: all clean

all: multi-lookup

multi-lookup: multi-lookup.o util.o
	$(CC) $(LFLAGS) $^ -o $@

util.o: util.c util.h
	$(CC) $(CFLAGS) $<

multi-lookup.o: multi-lookup.c multi-lookup.h
	$(CC) $(CFLAGS) $<

clean:
	rm -f multi-lookup 
	rm -f *.o
	rm -f *~
