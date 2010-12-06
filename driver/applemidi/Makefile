
PROJECTDIR=../..
SUBDIR=driver/applemidi

include ../../config.mk

LDFLAGS := $(LDFLAGS) -lmidikit

OBJS=$(OBJDIR)/applemidi.o
LIB=$(LIBDIR)/libmidikit-applemidi$(LIB_SUFFIX)

.PHONY: all clean $(LIB)

all: $(LIB)

clean:
	rm -f $(LIB)
	rm -f $(OBJS)

$(OBJDIR)/%.o:
	@$(MKDIR_P) $(OBJDIR)
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(LIB): $(OBJS)
	$(CC) $(LDFLAGS_LIB) -o $@ $^

$(OBJDIR)/applemidi.o: applemidi.c applemidi.h
