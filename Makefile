# Neatcc top-level Makefile

# output architecture: x64, x86, arm
OUT = x64
CC = cc

BASE = $(PWD)

all: help

help:
	@echo "Neatcc top-level makefile"
	@echo
	@echo "   neat        Compile the programs"
	@echo "   boot        Compile neatcc using itself"
	@echo "   test        Run compiler tests"
	@echo "   pull        Update git repositories"
	@echo "   clean       Remove the generated files"
	@echo

pull:
	git pull

neat:
	@cd ldelf && $(MAKE)
	# compiling the programs
	@cd neatcc && $(MAKE) OUT=$(OUT) CC="$(CC)"
	@cd neatld && $(MAKE) OUT=$(OUT) CC="$(CC)"
	@cd neatlibc && $(MAKE) OUT=$(OUT) CC=../neatcc/ncc
	# setting up neatrun/neatcc
	@cd neatrun && $(MAKE) OUT=$(OUT) NCC=$(BASE)/neatcc/ncc \
		NLD=$(BASE)/neatld/nld NLC=$(BASE)/neatlibc clean all
	# compiling the rest
	#@cd neatas && $(MAKE) CC=../neatrun/neatcc OUT=$(OUT)
	@cd neatas && $(MAKE) OUT=$(OUT)
	@cd neatdbg && $(MAKE) CC=../neatrun/neatcc OUT=$(OUT)
	#@cd neatdbg && $(MAKE) OUT=$(OUT)
	# compiling demos
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
	@cd neatcc && $(MAKE) OUT=$(OUT) CC="../ldelf/ldelf ../_neatcc" clean all
	# 2222222222
	@cd neatld && $(MAKE) OUT=$(OUT) CC="../ldelf/ldelf ../_neatcc" clean all
	# 3333333333
	@cd neatlibc && $(MAKE) OUT=$(OUT) CC="../ldelf/ldelf ../neatcc/ncc" clean all
	# setting up neatrun/neatcc
	# 4444444444
	@cd neatrun && $(MAKE) OUT=$(OUT) CC="../ldelf/ldelf ../_neatcc" NCC=$(BASE)/neatcc/ncc \
		NLD=$(BASE)/neatld/nld NLC=$(BASE)/neatlibc clean all
	@rm _ncc _nld _neatcc
	# compiling the rest
	# 5555555555
	@cd neatas && $(MAKE) CC="../ldelf/ldelf ../neatrun/neatcc" OUT=$(OUT) clean all
	# 6666666666
	@cd neatdbg && $(MAKE) CC="../ldelf/ldelf ../neatrun/neatcc" OUT=$(OUT) clean all
	@echo DONE

.PHONY: test
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
	@cd demo && $(MAKE) clean
	@cd test && $(MAKE) clean
