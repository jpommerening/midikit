
PROJECTDIR=..

include $(PROJECTDIR)/config.mk

OBJDIR=.

CFLAGS := $(CFLAGS) -I..
LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lmidikit

OBJS=$(OBJDIR)/midi.o $(OBJDIR)/message.o $(OBJDIR)/clock.o
BIN=test_main

.PHONY: all clean $(LIB)

all: tests.passed

clean:
	rm -f $(OBJS)
	rm -f $(BIN)
	rm -f tests.passed $(BIN).c

$(OBJDIR)/%.o:
	@$(MKDIR_P) $(OBJDIR)
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(BINDIR)/$(BIN): main.c $(OBJS)
	$(CC) $(LDFLAGS_BIN) -o $@ $^

$(OBJDIR)/midi.o: midi.c test.h
$(OBJDIR)/message.o: message.c test.h
$(OBJDIR)/clock.o: clock.c test.h

tests.passed: $(BINDIR)/$(BIN)
	LD_LIBRARY_PATH=$(LIBDIR) $(BINDIR)/$(BIN) && touch $@

main.c: midi.c message.c clock.c
	./generate_main.sh $^
