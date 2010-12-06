
PROJECTDIR=..
SUBDIR=test

include $(PROJECTDIR)/config.mk

CFLAGS := $(CFLAGS)
LDFLAGS := $(LDFLAGS) -lmidikit

OBJS=$(OBJDIR)/midi.o $(OBJDIR)/util.o $(OBJDIR)/clock.o \
     $(OBJDIR)/message_format.o $(OBJDIR)/message.o \
     $(OBJDIR)/connector.o $(OBJDIR)/device.o $(OBJDIR)/driver.o \
     $(OBJDIR)/message_queue.o \
     $(OBJDIR)/integration.o
BIN=test_main

MAIN_C=main.c

.PHONY: all clean $(LIB)

all: tests.passed

clean:
	rm -f $(OBJS)
	rm -f $(BIN)
	rm -f tests.passed $(MAIN_C)

$(OBJDIR)/%.o:
	@$(MKDIR_P) $(OBJDIR)
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(BINDIR)/$(BIN): $(MAIN_C) $(OBJS)
	$(CC) $(LDFLAGS_BIN) -o $@ $^

$(OBJDIR)/midi.o: midi.c test.h
$(OBJDIR)/util.o: util.c test.h
$(OBJDIR)/clock.o: clock.c test.h
$(OBJDIR)/message_format.o: message_format.c test.h
$(OBJDIR)/message.o: message.c test.h
$(OBJDIR)/connector.o: connector.c test.h
$(OBJDIR)/device.o: device.c test.h
$(OBJDIR)/driver.o: driver.c test.h
$(OBJDIR)/message_queue.o: message_queue.c test.h
$(OBJDIR)/integration.o: integration.c test.h

tests.passed: $(BINDIR)/$(BIN)
	LD_LIBRARY_PATH=$(LIBDIR) $(BINDIR)/$(BIN) && touch $@

$(MAIN_C): midi.c util.c clock.c message_format.c message.c connector.c device.c driver.c integration.c
	./generate_main.sh -o $(MAIN_C) $^
