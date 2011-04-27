
PROJECTDIR=..
SUBDIR=midi

include ../config.mk

OBJS=$(OBJDIR)/midi.o $(OBJDIR)/util.o $(OBJDIR)/event.o $(OBJDIR)/list.o \
     $(OBJDIR)/message.o $(OBJDIR)/message_format.o $(OBJDIR)/port.o \
     $(OBJDIR)/clock.o $(OBJDIR)/driver.o $(OBJDIR)/device.o \
     $(OBJDIR)/controller.o $(OBJDIR)/timer.o \
     $(OBJDIR)/runloop.o $(OBJDIR)/message_queue.o
LIB_NAME=libmidikit
LIB=$(LIBDIR)/$(LIB_NAME)$(LIB_SUFFIX)

.PHONY: all clean

all: $(LIB)

clean:
	rm -f $(LIB)
	rm -f $(OBJS)

$(OBJDIR)/%.o:
	@$(MKDIR_P) $(OBJDIR)
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(LIBDIR)/$(LIB_NAME)$(LIB_SUFFIX_SHARED): $(OBJS)
	$(CC) $(LDFLAGS_LIB) -o $@ $^

$(LIBDIR)/$(LIB_NAME)$(LIB_SUFFIX_STATIC): $(OBJS)
	$(AR) rs $@ $^

$(OBJDIR)/cfintegration.o: cfintegration.c
$(OBJDIR)/clock.o: clock.c clock.h midi.h
$(OBJDIR)/controller.o: controller.c device.h midi.h controller.h
$(OBJDIR)/device.o: device.c device.h midi.h message.h clock.h port.h controller.h timer.h
$(OBJDIR)/driver.o: driver.c runloop.h driver.h midi.h clock.h list.h message.h port.h
$(OBJDIR)/event.o: event.c event.h midi.h type.h
$(OBJDIR)/list.o: list.c midi.h list.h
$(OBJDIR)/message.o: message.c message.h midi.h clock.h message_format.h type.h
$(OBJDIR)/message_format.o: message_format.c message_format.h midi.h
$(OBJDIR)/message_queue.o: message_queue.c message_queue.h midi.h message.h clock.h
$(OBJDIR)/midi.o: midi.c midi.h
$(OBJDIR)/port.o: port.c midi.h list.h port.h type.h
$(OBJDIR)/runloop.o: runloop.c runloop.h midi.h
$(OBJDIR)/timer.o: timer.c midi.h timer.h device.h clock.h message.h
$(OBJDIR)/util.o: util.c util.h midi.h driver.h device.h port.h
