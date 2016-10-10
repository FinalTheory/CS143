# Cool Project

This project is out of personal interest, its final goal is to build a self-made webserver, which rely on a self-made TCP/IP stack, which works on a self-made operating system, which is running on a self-made CPU using simple "Y86" instruction set, composed with an improved version of [Cool Language](https://en.wikipedia.org/wiki/Cool_(programming_language)), compiled with a self-made Cool compiler, and assembled with a self-made Y86 assembler.

## CPU

Still working on.


## Compiler

### Requirements

- Flex
- Bison
- [Spim](http://spimsimulator.sourceforge.net) (a MIPS simulator)

For macOS, you could use
```
brew install flex bison spim
```
to install all these dependencies.


### Build

```
mkdir build && cd build
cmake ../
make
```


### Running

```
./coolc example.cl -o example.s
```
