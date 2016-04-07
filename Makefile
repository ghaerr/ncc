# Neatcc top-level Makefile

# output architecture: x64, x86, arm
OUT = x64
CC = cc

BASE = $(PWD)

all: help

help:
	@echo "Neatcc top-level makefile"
	@echo
	@echo "   init        Initialise git repositories"
	@echo "   neat        Compile the programs"
	@echo "   pull        Update git repositories"
	@echo "   clean       Remove the generated files"
	@echo

init:
	@test -d neatcc || git clone git://repo.or.cz/neatcc.git
	@test -d neatld || git clone git://repo.or.cz/ld.git neatld
	@test -d neatlibc || git clone git://repo.or.cz/neatlibc.git
	@test -d neatas || git clone git://repo.or.cz/neatas.git

pull:
	cd neatcc && git pull
	cd neatld && git pull
	cd neatlibc && git pull
	cd neatas && git pull
	git pull

neat:
	# compilation the programs
	@cd neatcc && $(MAKE) OUT=$(OUT) CC=$(CC)
	@cd neatld && $(MAKE) OUT=$(OUT) CC=$(CC)
	@cd neatlibc && $(MAKE) OUT=$(OUT) CC=../neatcc/ncc
	@cd neatrun && $(MAKE) OUT=$(OUT) CC=$(CC) \
		NCC=../_ncc NLD=../_nld NLC=../neatlibc
	# bootstrapping
	@cp neatcc/ncc _ncc
	@cp neatld/nld _nld
	@cp neatrun/neatcc _neatcc
	@cd neatcc && $(MAKE) OUT=$(OUT) CC=../_neatcc clean all
	@cd neatlibc && $(MAKE) OUT=$(OUT) CC=../neatcc/ncc clean all
	@cd neatld && $(MAKE) OUT=$(OUT) CC=../_neatcc clean all
	# setting up neatrun/neatcc
	@cd neatrun && $(MAKE) OUT=$(OUT) CC=../_neatcc NCC=$(BASE)/neatcc/ncc \
		NLD=$(BASE)/neatld/nld NLC=$(BASE)/neatlibc clean all
	@rm _ncc _nld _neatcc
	# compiling the rest
	@cd neatas && $(MAKE) CC=../neatrun/neatcc OUT=$(OUT)
	@cd neatdbg && $(MAKE) CC=../neatrun/neatcc OUT=$(OUT)

clean:
	@cd neatcc && $(MAKE) clean
	@cd neatlibc && $(MAKE) clean
	@cd neatas && $(MAKE) clean
	@cd neatld && $(MAKE) clean
	@cd neatrun && $(MAKE) clean
	@cd neatdbg && $(MAKE) clean
