
PROJECTDIR=.

include config.mk

default: all

all: midi driver test

midi: midi/.make
driver: driver/.make
test: test/.make

driver/.make: midi
test/.make: midi

%/.make:
	@cd $$(dirname $@) && $(MAKE)


