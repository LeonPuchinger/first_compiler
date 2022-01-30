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

symbol:
	$(BUILDSTR) -c $(SRC)/symbol.c -o $(BIN)/symbol.o

# use partial linking to generate a large object file called "compiler_artifact.o".
# it contains all the compiler steps and can be used to easily link all the necessary
# parts to a single test file (which often requires all compiler steps).
# subsequently link the "compiler_artifact.o" with the compiled version of "main.c"
# to generate the actual compiler executable.
compiler: lexer parser symbol
	ld -r $(BIN)/lexer.o $(BIN)/parser.o $(BIN)/symbol.o -o bin/compiler_artifact.o
	$(BUILDSTR) $(SRC)/main.c $(BIN)/compiler_artifact.o -o $(BIN)/compiler
