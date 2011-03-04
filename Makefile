CC = cc
CFLAGS = -Wall -Os
LDFLAGS =

all: neatas
.c.o:
	$(CC) -c $(CFLAGS) $<
neatas: neatas.o out.o
	$(CC) -o $@ $^ $(LDFLAGS)
clean:
	rm -f neatas *.o
