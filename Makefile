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
	@echo "   boot        Compile neatcc using itself"
	@echo "   pull        Update git repositories"
	@echo "   clean       Remove the generated files"
	@echo

init:
	@test -d neatcc || git clone git://github.com/aligrudi/neatcc.git
	@test -d neatld || git clone git://github.com/aligrudi/neatld.git neatld
	@test -d neatlibc || git clone git://github.com/aligrudi/neatlibc.git
	@test -d neatas || git clone git://repo.or.cz/neatas.git

pull:
	cd neatcc && git pull
	cd neatld && git pull
	cd neatlibc && git pull
	cd neatas && git pull
	git pull

neat:
	# compiling the programs
	@cd neatcc && $(MAKE) OUT=$(OUT) CC=$(CC)
	@cd neatld && $(MAKE) OUT=$(OUT) CC=$(CC)
	@cd neatlibc && $(MAKE) OUT=$(OUT) CC=../neatcc/ncc
	# setting up neatrun/neatcc
	@cd neatrun && $(MAKE) OUT=$(OUT) NCC=$(BASE)/neatcc/ncc \
		NLD=$(BASE)/neatld/nld NLC=$(BASE)/neatlibc clean all
	# compiling the rest
	@cd neatas && $(MAKE) CC=../neatrun/neatcc OUT=$(OUT)
	@cd neatdbg && $(MAKE) CC=../neatrun/neatcc OUT=$(OUT)

boot:
	# the previous version
	@cp neatrun/neatcc _neatcc
	@cd neatrun && $(MAKE) OUT=$(OUT) CC=../_neatcc \
		NCC=../_ncc NLD=../_nld NLC=../neatlibc clean all
	@cp neatcc/ncc _ncc
	@cp neatld/nld _nld
	@cp neatrun/neatcc _neatcc
	# compiling the programs
	@cd neatcc && $(MAKE) OUT=$(OUT) CC=../_neatcc clean all
	@cd neatld && $(MAKE) OUT=$(OUT) CC=../_neatcc clean all
	@cd neatlibc && $(MAKE) OUT=$(OUT) CC=../neatcc/ncc clean all
	# setting up neatrun/neatcc
	@cd neatrun && $(MAKE) OUT=$(OUT) CC=../_neatcc NCC=$(BASE)/neatcc/ncc \
		NLD=$(BASE)/neatld/nld NLC=$(BASE)/neatlibc clean all
	@rm _ncc _nld _neatcc
	# compiling the rest
	@cd neatas && $(MAKE) CC=../neatrun/neatcc OUT=$(OUT) clean all
	@cd neatdbg && $(MAKE) CC=../neatrun/neatcc OUT=$(OUT) clean all

clean:
	@cd neatcc && $(MAKE) clean
	@cd neatlibc && $(MAKE) clean
	@cd neatas && $(MAKE) clean
	@cd neatld && $(MAKE) clean
	@cd neatrun && $(MAKE) clean
	@cd neatdbg && $(MAKE) clean
