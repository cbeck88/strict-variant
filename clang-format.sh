#!/bin/bash
BIN=~/llvm-src/llvm/build/bin/clang-format
"$BIN" --style=file -i `find include/ -name *.?pp`
"$BIN" --style=file -i `find test/include -name *.?pp`
"$BIN" --style=file -i `find test -name *.cpp`
