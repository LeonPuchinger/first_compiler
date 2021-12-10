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

build: compiler

lexer:
	$(BUILDSTR) -c $(SRC)/lexer.c -o $(BIN)/lexer.o

parser:
	$(BUILDSTR) -c $(SRC)/parser.c -o $(BIN)/parser.o

compiler: lexer parser
	$(BUILDSTR) $(SRC)/main.c $(BIN)/lexer.o $(BIN)/parser.o -o $(BIN)/compiler
