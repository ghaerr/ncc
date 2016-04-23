CC = cc
CFLAGS = -O2 -Wall
LDFLAGS =

all: elfloc coredump
%.o: %.c
	$(CC) -c $(CFLAGS) $<
elfloc: elfloc.o
	$(CC) $(LDFLAGS) -o $@ elfloc.o
coredump: coredump.o
	$(CC) $(LDFLAGS) -o $@ coredump.o
clean:
	rm -f *.o elfloc coredump
