
CC = gcc
CFLAGS = -O3 -Wall -I..
CFLAGS_OBJ = $(CFLAGS) -c
LIBDIR=../midi
LDFLAGS = -L$(LIBDIR) -lmidikit
LDFLAGS_BIN = $(LDFLAGS)

OBJS=midi.o message.o clock.o
BIN=main

.PHONY: all clean $(LIB)

all: tests.passed

clean:
	rm -f $(OBJS)
	rm -f $(BIN)
	rm -f tests.passed $(BIN).c

%.o:
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(BIN): $(BIN).c $(OBJS)
	$(CC) $(LDFLAGS_BIN) -o $@ $^

midi.o: midi.c test.h
message.o: message.c test.h
clock.o: clock.c test.h

tests.passed: $(BIN)
	LD_LIBRARY_PATH=$(LIBDIR) ./$(BIN) && touch $@

$(BIN).c: midi.c message.c clock.c
	./generate_main.sh $^
