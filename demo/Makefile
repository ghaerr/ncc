CC = ../neatrun/neatcc
LD = ../neatld/nld

CFLAGS = -Wall -O2
LDFLAGS = -p

ifeq ($(LOADER), )
#LOADER = ../ldelf/ldelf
LOADER = ../ldelf/load
endif

all: test

%.o: %.c
	$(CC) -c $(CFLAGS) $<
test: test.o
	$(CC) -o $@ $^ $(LDFLAGS)
	$(LOADER) ./test
test2: test2.o ttyname.o
	$(CC) -o $@ $^ $(LDFLAGS)
	$(LOADER) ./test2 these are args
clean:
	rm -f *.o test test2
