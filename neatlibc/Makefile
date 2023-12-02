# output architecture: x64, x86, arm
OUT = x64

# platform uname for syscalls
PLATFORM=$(shell uname)

# loader path for execve on macOS
LOADER=$(shell dirname $(shell pwd))/ldelf/ldelf

# default assemblers
ASx64 = nasm -f elf64 -d$(PLATFORM)
ASx86 = nasm -f elf
ASarm = neatas

CC = ../neatcc/ncc
AS = $(AS$(OUT))
CFLAGS = -O2 -I. -DLOADER='"$(LOADER)"'

all: start.o libc.a

%.o: %.s
	$(AS) $^ >/dev/null
%.o: %.c
	$(CC) -c $(CFLAGS) $^

SSRC = $(filter-out $(OUT)/start.o $(wildcard $(OUT)/syscall*.s), $(wildcard $(OUT)/*.s))
SSRC += $(OUT)/syscall-$(shell uname).o
SOBJS = $(patsubst %.s,%.o,$(SSRC))
COBJS = $(patsubst %.c,%.o,$(wildcard *.c))
OBJS = $(COBJS) $(SOBJS)

start.o: $(OUT)/start.o
	cp $(OUT)/start.o .
libc.a: $(OBJS)
	#$(AR) rcs $@ $(OBJS)
	$(AR) rcs $@ `lorder $(OBJS) | tsort -q`

clean:
	rm -f *.o *.a x86/*.o arm/*.o x64/*.o
