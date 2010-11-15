
PROJECTDIR=..

include ../config.mk

OBJS=$(OBJDIR)/midi.o $(OBJDIR)/message.o $(OBJDIR)/message_format.o \
     $(OBJDIR)/clock.o $(OBJDIR)/input.o $(OBJDIR)/output.o \
     $(OBJDIR)/driver.o $(OBJDIR)/device.o
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
$(OBJDIR)/message.o: message.c message.h
$(OBJDIR)/message_format.o: message_format.c message_format.h
$(OBJDIR)/clock.o: clock.c clock.h
$(OBJDIR)/input.o: input.c input.h device.h driver.h
$(OBJDIR)/output.o: output.c output.h device.h driver.h
$(OBJDIR)/driver.o: driver.c driver.h message.h input.h output.h
$(OBJDIR)/device.o: device.c device.h message.h input.h output.h

