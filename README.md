# lllisp

A pure lisp compiler with LLVM

## Build

It requires Clang (to compile lib.c to lib.ll)

    $ make

## Usage

    $ ./compile FILE
    $ ./a.out

or manually

    $ ./lisp < FILE                     # lisp source to LLVM IR (emit to a.bc)
    $ llvm-link a.bc lib.ll -S -o a2.bc # link lib.ll and a.bc
    $ llc a2.bc                         # LLVM IR to assembly of target architecture
    $ clang a2.s                        # assembly to machine code
