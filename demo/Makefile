CC = ../neatrun/neatcc
LD = ../neatld/nld

CFLAGS = -Wall -O2
LDFLAGS = -p

# ldtest load address must be below normal load address of 0x400000 (4MB)
LOADADDR = -mc=0x200000     # 2MB


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

ldtest: ldtest.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LOADADDR)
	$(LOADER) ./ldtest

clean:
	rm -f *.o test test2 ldtest
