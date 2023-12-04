# Makefile for sash

CC = ../neatrun/neatcc
LD = ../neatrun/neatcc
CFLAGS += -O2
xCFLAGS += -Os -Wno-implicit-int -Wall

OBJS = sash.o cmds.o cmd_dd.o cmd_ed.o cmd_grep.o cmd_ls.o cmd_tar.o utils.o cmd_history.o
#LDFLAGS = -lc

all: sash

sash:	$(OBJS)
	$(LD) $(LDFLAGS) -o sash $(OBJS) $(LDLIBS)

clean:
	rm -f core sash $(OBJS)

$(OBJS): sash.h config.h
