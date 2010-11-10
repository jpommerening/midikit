
CC = gcc
CFLAGS = -O3 -Wall
CFLAGS_OBJ = $(CFLAGS) -c
LDFLAGS =
LDFLAGS_BIN = $(LDFLAGS)
LDFLAGS_LIB = $(LDFLAGS) -shared

DRIVERS=rtp-midi osc

default: all

all: midi test

midi:
	cd midi && $(MAKE)

test:
	cd test && $(MAKE)

$(DRIVERS): 
	cd driver/$@ && $(MAKE)

