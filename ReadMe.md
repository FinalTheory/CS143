# Cool Project [![Build Status](https://travis-ci.org/FinalTheory/Cool.svg?branch=master)](https://travis-ci.org/FinalTheory/Cool)

This project is out of my personal interest, its final goal is to build a self-made webserver, which uses a self-made TCP/IP stack, which works on a self-made operating system, which is running on a self-made CPU using simple "Y86" instruction set, built with an improved version of [Cool Language](https://en.wikipedia.org/wiki/Cool_(programming_language)), compiled by a self-made Cool compiler, and assembled by a self-made Y86 assembler.

## CPU

Still working on.


## Compiler

Notice that current version of this compiler generates MIPS code only, supports for Y86 machine code is not implemented yet.


### Requirements

- cmake
- Perl
- Flex
- Bison
- [Spim](http://spimsimulator.sourceforge.net) (a MIPS simulator)

For macOS, you could use
```
brew install cmake flex bison spim perl
```
to install all these dependencies.


### Build


To build cool compiler:
```
mkdir build && cd build
cmake ../
make
```

To run functionality tests:
```
make cooltest
```


### Running

```
coolc example.cl -o example.s
spim -trap_file runtime/mips.handler -file example.s
```
Notice that on macOS, spim may give some warnings like:
```
The following symbols are undefined:
main
```
and this could be safely ignored.


### Reference

- [[Cool Manual](http://theory.stanford.edu/~aiken/software/cool/cool-manual.pdf)] Cool language spec and implementation details.
