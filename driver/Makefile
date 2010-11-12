 
PROJECTDIR=..

include ../config.mk

.PHONY: all clean $(DRIVERS)

all: $(DRIVERS)

clean:

generic: generic/.make
apple-midi: apple-midi/.make
osc: osc/.make

%/.make:
	@echo "cd $$(dirname $@) && $(MAKE)"
