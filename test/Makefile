
PROJECTDIR=..
SUBDIR=test

include $(PROJECTDIR)/config.mk

CFLAGS := $(CFLAGS)
LDFLAGS_SHARED := $(LDFLAGS) -lmidikit -lmidikit-driver
LDFLAGS_STATIC := $(LDFLAGS) $(LIBDIR)/libmidikit$(LIB_SUFFIX_STATIC) $(LIBDIR)/libmidikit-driver$(LIB_SUFFIX_STATIC)
LDFLAGS := $(LDFLAGS_$(COMPILE_MODE))

OBJS=$(OBJDIR)/midi.o $(OBJDIR)/util.o $(OBJDIR)/list.o $(OBJDIR)/port.o \
     $(OBJDIR)/clock.o $(OBJDIR)/message_format.o $(OBJDIR)/message.o \
     $(OBJDIR)/device.o $(OBJDIR)/driver.o $(OBJDIR)/message_queue.o \
     $(OBJDIR)/integration.o $(OBJDIR)/runloop.o \
     $(OBJDIR)/driver_rtp.o $(OBJDIR)/driver_applemidi.o
BIN=test_main

MAIN_C=main.c

.PHONY: all clean

all: tests.passed

clean:
	rm -f $(OBJS)
	rm -f $(BIN)
	rm -f tests.passed $(MAIN_C)

$(OBJDIR)/%.o:
	@$(MKDIR_P) $(OBJDIR)
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(BINDIR)/$(BIN): $(MAIN_C) $(OBJS)
	$(CC) $^ $(LDFLAGS_BIN) $(CFLAGS) -o $@

$(BINDIR)/demo: demo.c
	$(CC) $(LDFLAGS_BIN) -o $@ $^

$(OBJDIR)/midi.o: midi.c test.h
$(OBJDIR)/util.o: util.c test.h
$(OBJDIR)/list.o: list.c test.h
$(OBJDIR)/clock.o: clock.c test.h
$(OBJDIR)/message_format.o: message_format.c test.h
$(OBJDIR)/message.o: message.c test.h
$(OBJDIR)/controller.o: controller.c test.h
$(OBJDIR)/device.o: device.c test.h
$(OBJDIR)/driver.o: driver.c test.h
$(OBJDIR)/message_queue.o: message_queue.c test.h
$(OBJDIR)/port.o: port.c test.h
$(OBJDIR)/integration.o: integration.c test.h
$(OBJDIR)/runloop.o: runloop.c test.h
$(OBJDIR)/driver_rtp.o: driver_rtp.c test.h
$(OBJDIR)/driver_applemidi.o: driver_applemidi.c test.h

tests.passed: $(BINDIR)/$(BIN) $(LIBDIR)/libmidikit$(LIB_SUFFIX) $(LIBDIR)/libmidikit-driver$(LIB_SUFFIX)
	LD_LIBRARY_PATH=$(LIBDIR) valgrind --leak-check=full $(BINDIR)/$(BIN) && touch $@

$(MAIN_C): midi.c util.c list.c port.c clock.c message_format.c message.c device.c driver.c integration.c runloop.c driver_rtp.c driver_applemidi.c
	./generate_main.sh -o $(MAIN_C) $^
