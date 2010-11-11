
CC = gcc
CFLAGS = -O3 -Wall
CFLAGS_OBJ = $(CFLAGS) -c
LDFLAGS =
LDFLAGS_LIB = $(LDFLAGS) -shared

OBJS=midi.o message.o clock.o input.o output.o driver.o device.o
LIB=libmidikit.so

.PHONY: all clean $(LIB)

all: $(LIB)

clean:
	rm -f $(LIB)
	rm -f $(OBJS)

%.o:
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(LIB): $(OBJS)
	$(CC) $(LDFLAGS_LIB) -o $@ $^

midi.o: midi.c midi.h
message.o: message.c message.h
clock.o: clock.c clock.h
input.o: input.c input.h device.h driver.h
output.o: output.c output.h device.h driver.h
driver.o: driver.c driver.h message.h input.h output.h
device.o: device.c device.h message.h input.h output.h

