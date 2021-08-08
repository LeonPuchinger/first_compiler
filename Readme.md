# first compiler

my first attempt at building a compiler from scratch. compiles a very simple language to x86 machine code.

```
//language looks something like this
function abc {
    x1 = 1 + 2
    x2 = x1 + x2
}

abc()
```

## limitations

- can only compute integers
- functions do not have arguments
- no control structures like loops/conditions/...
