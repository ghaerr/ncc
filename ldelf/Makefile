# ldelf - elf loader

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
        -e start                        \
        -pagezero_size 0x100000         \

OBJS = ldelf.o systemcall.o launch.o start.o
OBJS2 = load.o systemcall.o launch.o start.o

all: ldelf load

%.o: %.c
	$(CC) -c $(CFLAGS) $<

ldelf: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

load: $(OBJS2)
	$(CC) -o $@ $(LDFLAGS) $(OBJS2)

clean:
	rm -f *.o ldelf load
