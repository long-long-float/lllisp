# lllisp

A pure lisp compiler with LLVM

## Build

It requires Clang (to compile lib.c to lib.ll)

    $ make

## Usage

    $ ./compile FILE
    $ ./a.out

or manually

    $ ./lisp < FILE
    $ llvm-link a.bc lib.ll -S -o a2.bc
    $ llc a2.bc
    $ g++ a2.s
