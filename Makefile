# NCC top-level Makefile

# output architecture: x64, x86, arm
OUT = x64
CC = cc

BASE = $(PWD)
#LOADER = $(BASE)/ldelf/ldelf
#BOOTLDR = ../ldelf/ldelf
LOADER = $(BASE)/ldelf/load
BOOTLDR = ../ldelf/load
export LOADER

all: help

help:
	@echo "NCC top-level makefile"
	@echo
	@echo "   neat        Compile the programs"
	@echo "   sash        Compile and run standalone shell"
	@echo "   boot        Compile neatcc using itself"
	@echo "   test        Run compiler tests"
	@echo "   clean       Remove the generated files"
	@echo

neat:
	@cd ldelf && $(MAKE)
	# compiling the programs
	@cd neatcc && $(MAKE) OUT=$(OUT) CC="$(CC)"
	@cd neatld && $(MAKE) OUT=$(OUT) CC="$(CC)"
	@cd neatlibc && $(MAKE) OUT=$(OUT) CC=../neatcc/ncc
	# setting up neatrun/neatcc
	@cd neatrun && $(MAKE) OUT=$(OUT) NCC=$(BASE)/neatcc/ncc \
		NLD=$(BASE)/neatld/nld NLC=$(BASE)/neatlibc all
	# compiling the rest
	@cd neatas && $(MAKE) OUT=$(OUT)
	@cd neatdbg && $(MAKE) OUT=$(OUT)
	# compiling demos and sash
	@cd sash && $(MAKE)
	@cd demo && $(MAKE)

boot:
	# the previous version
	@cp neatrun/neatcc _neatcc
	# 0000000000
	@cd neatrun && $(MAKE) OUT=$(OUT) CC=../_neatcc \
		NCC=../_ncc NLD=../_nld NLC=../neatlibc clean all
	@cp neatcc/ncc _ncc
	@cp neatld/nld _nld
	@cp neatrun/neatcc _neatcc
	# compiling the programs
	# 1111111111
	@cd neatcc && $(MAKE) OUT=$(OUT) CC="$(BOOTLDR) ../_neatcc" clean all
	# 2222222222
	@cd neatld && $(MAKE) OUT=$(OUT) CC="$(BOOTLDR) ../_neatcc" clean all
	# 3333333333
	@cd neatlibc && $(MAKE) OUT=$(OUT) CC="$(BOOTLDR) ../neatcc/ncc" clean all
	# setting up neatrun/neatcc
	# 4444444444
	@cd neatrun && $(MAKE) OUT=$(OUT) CC="$(BOOTLDR) ../_neatcc" NCC=$(BASE)/neatcc/ncc \
		NLD=$(BASE)/neatld/nld NLC=$(BASE)/neatlibc clean all
	@rm _ncc _nld _neatcc
	# compiling the rest
	# 5555555555
	@cd neatas && $(MAKE) CC="$(BOOTLDR) ../neatrun/neatcc" OUT=$(OUT) clean all
	# 6666666666
	@cd neatdbg && $(MAKE) CC="$(BOOTLDR) ../neatrun/neatcc" OUT=$(OUT) clean all
	@echo DONE

.PHONY: sash test libc demo
sash:
	@cd sash && $(MAKE)
	$(LOADER) sash/sash

libc:
	@cd neatlibc && $(MAKE)

demo:
	@cd demo && $(MAKE)
	$(LOADER) demo/test

test:
	@cd test && $(MAKE)

clean:
	@cd neatcc && $(MAKE) clean
	@cd neatlibc && $(MAKE) clean
	@cd neatas && $(MAKE) clean
	@cd neatld && $(MAKE) clean
	@cd neatrun && $(MAKE) clean
	@cd neatdbg && $(MAKE) clean
	@cd ldelf && $(MAKE) clean
	@cd sash && $(MAKE) clean
	@cd demo && $(MAKE) clean
	@cd test && $(MAKE) clean
