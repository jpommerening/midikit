 
PROJECTDIR=..

include ../config.mk

.PHONY: all clean $(DRIVERS)

all: $(DRIVERS)

clean:

generic: generic/.make
applemidi: applemidi/.make
osc: osc/.make

%/.make:
	@echo "cd $$(dirname $@) && $(MAKE)"
