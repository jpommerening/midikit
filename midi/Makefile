
PROJECTDIR=..
SUBDIR=midi

include ../config.mk

OBJS=$(OBJDIR)/midi.o $(OBJDIR)/message.o $(OBJDIR)/message_format.o \
     $(OBJDIR)/clock.o $(OBJDIR)/connector.o  $(OBJDIR)/driver.o \
     $(OBJDIR)/device.o $(OBJDIR)/controller.o $(OBJDIR)/timer.o
LIB=$(LIBDIR)/libmidikit$(LIB_SUFFIX)

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

$(OBJDIR)/midi.o: midi.c midi.h
$(OBJDIR)/clock.o: clock.c clock.h
$(OBJDIR)/message_format.o: message_format.c message_format.h midi.h
$(OBJDIR)/message.o: message.c message.h midi.h clock.h message_format.h
$(OBJDIR)/connector.o: connector.c connector.h
$(OBJDIR)/device.o: device.c device.h midi.h message.h clock.h connector.h
$(OBJDIR)/driver.o: driver.c driver.h midi.h message.h clock.h connector.h
$(OBJDIR)/controller.o: controller.c controller.h midi.h device.h
$(OBJDIR)/timer.o: timer.c timer.h midi.h device.h
