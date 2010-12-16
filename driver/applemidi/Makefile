
PROJECTDIR=../..
SUBDIR=driver/applemidi

include ../../config.mk

LDFLAGS := $(LDFLAGS) -lmidikit

OBJS=$(OBJDIR)/applemidi.o

.PHONY: all clean

all: $(OBJS)

clean:
	rm -f $(LIB)
	rm -f $(OBJS)

$(OBJDIR)/%.o:
	@$(MKDIR_P) $(OBJDIR)
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(OBJDIR)/applemidi.o: applemidi.c applemidi.h
