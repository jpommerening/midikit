
PROJECTDIR=.
SUBDIR=.

include config.mk

.PHONY: all clean midi driver test

default: all

all: midi driver test
clean: midi-clean driver-clean test-clean

documentation: midi driver
	$(MKDIR_P) documentation
	$(MKDIR_P) documentation/wiki
	doxygen
	if test x`which doxygen2gwiki 2>/dev/null` = "x"; then : ; else \
	  doxygen2gwiki -d documentation/xml -o documentation/wiki -p MIDIKit -s ; \
	fi

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


