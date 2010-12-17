 
PROJECTDIR=..
SUBDIR=driver

include ../config.mk

LDFLAGS := $(LDFLAGS) -lmidikit

OBJS_COMMON=$(OBJDIR)/common/rtp.o
OBJS_APPLEMIDI=$(OBJDIR)/applemidi/applemidi.o
OBJS_OSC=$(OBJDIR)/osc/osc.o

LIB=$(LIBDIR)/libmidikit-driver$(LIB_SUFFIX)

.PHONY: all clean common $(DRIVERS)

all: $(LIB)

clean:

common: $(OBJS_COMMON)
applemidi: $(OBJS_APPLEMIDI)
osc: $(OBJS_OSC)

$(OBJS_COMMON): common/.make
$(OBJS_APPLEMIDI): applemidi/.make
$(OBJS_OSC): osc/.make

$(LIB): $(OBJS_COMMON) $(OBJS_APPLEMIDI) $(OBJS_OSC)
	$(CC) $(LDFLAGS_LIB) -o $@ $^

%/.make:
	cd $$(dirname $@) && $(MAKE)
