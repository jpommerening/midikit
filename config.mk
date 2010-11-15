
MKDIR_P = mkdir -p
LN_S = ln -s

CC = clang
CFLAGS = -O3 -Wall
CFLAGS_OBJ = $(CFLAGS) -c
LDFLAGS =
LDFLAGS_LIB = $(LDFLAGS) -shared
LDFLAGS_BIN = $(LDFLAGS)

LIB_SUFFIX = .so
BIN_SUFFIX =

OBJDIR := .obj
LIBDIR := $(PROJECTDIR)
BINDIR := $(PROJECTDIR)

DRIVERS=generic apple-midi osc

