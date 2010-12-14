
PROJECTDIR=../..
SUBDIR=driver/common

include ../../config.mk

OBJS=$(OBJDIR)/rtp.o

.PHONY: all clean $(LIB)

all: $(OBJS)

clean:
	rm -f $(LIB)
	rm -f $(OBJS)

$(OBJDIR)/%.o:
	@$(MKDIR_P) $(OBJDIR)
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(OBJDIR)/rtp.o: rtp.c rtp.h
