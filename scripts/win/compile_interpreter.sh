#!/bin/sh

rm -r bin/build/interp/win
mkdir -p bin/build/interp/win

clang++ \
-Ofast \
# -Wno-everything \
-std=c++20 \
src/Node/Node.cpp \
src/Lexer/Lexer.cpp \
src/Parser/Parser.cpp \
src/Bytecode/Bytecode.cpp \
src/Bytecode/Generator.cpp \
src/VirtualMachine/VirtualMachine.cpp \
src/utils/utils.cpp \
main.cpp \
-o bin/build/interp/win/vortex.exe