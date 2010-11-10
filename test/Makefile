
CC = gcc
CFLAGS = -O3 -Wall -I..
CFLAGS_OBJ = $(CFLAGS) -c
LIBDIR=../midi
LDFLAGS = -L$(LIBDIR) -lmidikit
LDFLAGS_BIN = $(LDFLAGS)

OBJS=message.o
BIN=main

.PHONY: all clean $(LIB)

all: tests.passed

clean:
	rm -f $(OBJS)
	rm -f $(BIN)

%.o:
	$(CC) $(CFLAGS_OBJ) -o $@ $<

$(BIN): $(BIN).c $(OBJS)
	$(CC) $(LDFLAGS_BIN) -o $@ $^

message.o: message.c test.h

tests.passed: $(BIN)
	LD_LIBRARY_PATH=$(LIBDIR) ./$(BIN) && touch $@

$(BIN).c: message.c
	./generate_main.sh $^
