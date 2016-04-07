NCC = "/path/to/neatcc/ncc"
NLD = "/path/to/neatld/nld"
NLC = "/path/to/neatlibc/"

CC = cc
CFLAGS = -DNCC=\"$(NCC)\" -DNLD=\"$(NLD)\" -DNLC=\"$(NLC)\"
LDFLAGS =

all: neatcc

%.o: %.c
	$(CC) $(CFLAGS) -c $^
neatcc: neatcc.o
	$(CC) -o $@ neatcc.o $(LDFLAGS)
clean:
	rm -f *.o neatcc
