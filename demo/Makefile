CC = ../neatrun/neatcc

CFLAGS = -Wall -O2
LDFLAGS =

all: test

%.o: %.c
	$(CC) -c $(CFLAGS) $<
test: test.o
	$(CC) -o $@ test.o $(LDFLAGS)
clean:
	rm -f *.o test
