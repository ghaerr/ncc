# elf loaders

CC = cc
CFLAGS =                                \
        -Os                             \
        -Wall                           \
        -Wextra                         \
        -pedantic-errors                \
        -fno-stack-check                \
        -fno-stack-protector            \

LDFLAGS =                               \
        -static                         \
        -nostdlib                       \
        -pagezero_size 0x100000         \

ifeq ($(shell uname), Darwin)
LDFLAGS += -e xnustart
endif

LDELF = ldelf.o systemcall.o launch.o start.o
LOAD = load.o systemcall.o launch.o start.o

all: ldelf load

%.o: %.c
	$(CC) -c $(CFLAGS) $<

ldelf: $(LDELF)
	$(CC) -o $@ $(LDFLAGS) $(LDELF)

load: $(LOAD)
	$(CC) -o $@ $(LDFLAGS) $(LOAD)

clean:
	rm -f *.o ldelf load
