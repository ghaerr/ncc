CC = ../neatrun/neatcc
LD = ../neatld/nld
LOADER = ../ldelf/ldelf

CFLAGS = -Wall -O2
LDFLAGS = -p

all: test

%.o: %.c
	$(CC) -c $(CFLAGS) $<
test: test.o
	$(CC) -o $@ $^ $(LDFLAGS)
	$(LOADER) ./test
test2: test2.o ttyname.o
	$(CC) -o $@ $^ $(LDFLAGS)
	$(LOADER) ./test2
clean:
	rm -f *.o test test2
