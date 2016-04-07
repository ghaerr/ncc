CC = neatcc
CFLAGS = -O2 -Wall
LDFLAGS =

all: elfloc
%.o: %.c
	$(CC) -c $(CFLAGS) $<
elfloc: elfloc.o
	$(CC) $(LDFLAGS) -o $@ $^
coredump: coredump.o
	$(CC) $(LDFLAGS) -o $@ $^
clean:
	rm -f *.o elfloc coredump
