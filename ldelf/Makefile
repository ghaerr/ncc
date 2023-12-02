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

all: ldelf

%.o: %.c
	$(CC) -c $(CFLAGS) $<

ldelf: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

clean:
	rm -f *.o ldelf
