CC := clang
SRC := src
# location of test binaries
TST_BIN := bin
# location of actual binaries/artifacts to test
ART_BIN := ../bin
DEBUG ?= 0
OPTIMIZE ?= 0
BUILDSTR = $(CC) $(CCFLAGS)

ifeq ($(DEBUG), 1)
CCFLAGS += -g
endif

ifeq ($(OPTIMIZE), 1)
CCFLAGS += -O3
endif

.PHONY = build_tests execute_tests run_tests

setup:
	mkdir -p $(TST_BIN)

test:
	$(BUILDSTR) -c $(SRC)/test.c -o $(TST_BIN)/test.o

test_symbol:
	$(BUILDSTR) -c $(SRC)/test_symbol.c -o $(TST_BIN)/test_symbol.o

# build_tests just compiles the tests
# execute_tests just executes them
# run_tests does both

build_tests: setup test test_symbol
	$(BUILDSTR) $(ART_BIN)/compiler_artifact.o $(TST_BIN)/test.o $(TST_BIN)/test_symbol.o -o $(TST_BIN)/test_symbol

execute_tests:
	./$(TST_BIN)/test_symbol

run_tests: build_tests execute_tests
