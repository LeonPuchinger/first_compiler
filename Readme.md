# first compiler

my first attempt at building a compiler from scratch. compiles a simple language to x86_64 machine code.

```
//language looks something like this
x1 = 1 + 2
function abc {
    if (x1 == 3) {
        x1 = x1 + 1
    } else {
        x1 = x1 + 2
    }

    x2 = x1 + 1
    x3 = x1 + x2
}

abc()
```

## Build & Run

built (and tested) for linux only

requires nasm, ld, clang to be installed

```
$ make
$ ./bin/compiler input_program
```

execute generated binary:

```
$ ./out/out
```

### Run Tests

```
$ cd test/
$ make run_tests
```

## Todo

- [x] conditions
- [ ] a way to print values
- [ ] a few optimizations in the generated code
- [ ] better error handling

## limitations

- can only compute integers
- functions do not have arguments
    
    workaround: emulate by setting variable in the parent scope before calling function
    
    ```
    param = 0
    function abc {
        //use param as if it were a function parameter...
    }

    param = 1
    abc()
    param = 2
    abc()
    ```

- no control structures other than conditions (like loops, ...)

    workaround for loops using conditions & functions:

    ```
    counter = 10
    function loop {
        if (counter != 0) {
            counter = counter - 1
            loop()
        }
        //do loop stuff with counter...
    }
    loop()
    ```

## known issues

- the parser has less than ideal error handling: the compiler can exit 0, even though the parser could not find/match a suitable production
- there is no way to print values yet

## additional notes

- compilers are short lived programs, which is why this one (like a lot of other compilers) does not free memory.
i made this decision during the project however, which is why earlier compiler steps still free memory or offer functions to do so.
- inspired by https://github.com/rui314/chibicc
