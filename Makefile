
PROJECTDIR=.
SUBDIR=.

include config.mk

.PHONY: all clean midi driver test

default: all

all: midi driver test
clean: midi-clean driver-clean test-clean

midi: midi/.make
midi-clean: midi/.make-clean
driver-clean: driver/.make-clean
test: test/.make
test-clean: test/.make-clean

driver/.make: midi
test/.make: midi

%/.make:
	cd $$(dirname $@) && $(MAKE)

%/.make-clean:
	cd $$(dirname $@) && $(MAKE) clean


