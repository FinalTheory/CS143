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

For Linux system, install dependencies referring to [`.travis.yml`](https://github.com/FinalTheory/Cool/blob/master/.travis.yml).


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
