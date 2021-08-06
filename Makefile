CC := clang
SRC := src
BIN := bin
DEBUG ?= 0
OPTIMIZE ?= 0
BUILDSTR = $(CC) $(CCFLAGS)

ifeq ($(DEBUG), 1)
CCFLAGS += -g
endif

ifeq ($(OPTIMIZE), 1)
CCFLAGS += -O3
endif

all: setup build

setup:
	mkdir -p $(BIN)

build: compiler lexer

compiler:
	$(BUILDSTR) $(SRC)/main.c -o $(BIN)/compiler

lexer:
	$(BUILDSTR) $(SRC)/lexer.c -o $(BIN)/lexer
