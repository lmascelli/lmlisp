# lmlisp
My implementation of a lisp interpreter

## Introduction / Disclamer
This is an attempt of making a lisp interpreter following the line suggested by
the tutorial [make a lisp](https://github.com/kanaka/mal). Although the tutorial
is the main guide for my project it will maybe not follow it totally so do not
relief on it as a c++ implementation of mal lisp dialect cause it can differ in
some aspects

## Build
### Requirement
* C++-20 compatible compiler
* CMake
* (optional) powershell

### build instructions (with CMake)
``` bash
git clone https://github.com/lmascelli/lmlisp
cd lmlisp
mkdir build
cd build
cmake ..
cmake --build .
```

### build instructions (with powershell script)
``` ps1
git clone https://github.com/lmascelli/lmlisp
cd ./lmlisp
./project.ps1 build
```
