
PROJECTDIR=.
SUBDIR=.

include config.mk

.PHONY: all clean midi midi-clean driver driver-clean test test-clean

default: all

all: midi driver test
clean: midi-clean driver-clean test-clean

documentation: midi driver
	doxygen
	cd documentation && ./generate_wikidoc.sh

midi: midi/.make
midi-clean: midi/.make-clean
driver: driver/.make
driver-clean: driver/.make-clean
test: test/.make
test-clean: test/.make-clean

driver/.make: midi
test/.make: midi

%/.make:
	cd $$(dirname $@) && $(MAKE)

%/.make-clean:
	cd $$(dirname $@) && $(MAKE) clean


