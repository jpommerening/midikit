 
PROJECTDIR=..
SUBDIR=driver

include ../config.mk

LDFLAGS := $(LDFLAGS) -lmidikit

OBJS_COMMON=$(OBJDIR)/common/rtp.o $(OBJDIR)/common/rtpmidi.o
OBJS_APPLEMIDI=$(OBJDIR)/applemidi/applemidi.o
OBJS_OSC=$(OBJDIR)/osc/osc.o

LIB_NAME=libmidikit-driver
LIB=$(LIBDIR)/$(LIB_NAME)$(LIB_SUFFIX)

.PHONY: all clean common $(DRIVERS)

all: $(LIB)

clean: common-clean applemidi-clean osc-clean
	rm $(LIB)

common: $(OBJS_COMMON)
common-clean: common/.make-clean
applemidi: $(OBJS_APPLEMIDI)
applemidi-clean: applemidi/.make-clean
osc: $(OBJS_OSC)
osc-clean: osc/.make-clean

$(OBJS_COMMON): common/.make
$(OBJS_APPLEMIDI): applemidi/.make
$(OBJS_OSC): osc/.make

$(LIBDIR)/$(LIB_NAME)$(LIB_SUFFIX_SHARED): $(OBJS_COMMON) $(OBJS_APPLEMIDI) $(OBJS_OSC)
	$(CC) $(LDFLAGS_LIB) -o $@ $^

$(LIBDIR)/$(LIB_NAME)$(LIB_SUFFIX_STATIC): $(OBJS_COMMON) $(OBJS_APPLEMIDI) $(OBJS_OSC)
	$(AR) rs $@ $^

%/.make:
	cd $$(dirname $@) && $(MAKE)

%/.make-clean:
	cd $$(dirname $@) && $(MAKE) clean
