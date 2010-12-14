 
PROJECTDIR=..
SUBDIR=driver

include ../config.mk

LDFLAGS := $(LDFLAGS) -lmidikit

LIB=$(LIBDIR)/libmidikit-driver$(LIB_SUFFIX)

.PHONY: all clean $(DRIVERS)

all: $(LIB)

clean:

common: common/.make
generic: generic/.make
applemidi: applemidi/.make
osc: osc/.make

$(LIB): common $(DRIVERS)
	$(CC) $(LDFLAGS_LIB) -o $@ $(OBJDIR)/*/*.o

%/.make:
	cd $$(dirname $@) && $(MAKE)
