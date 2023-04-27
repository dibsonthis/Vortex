#!/bin/sh

macBuild()
{
    clang++ \
    -O3 \
    -std=c++20 \
    -stdlib=libc++ \
    "$PWD"/src/Node/Node.cpp \
    "$PWD"/src/Lexer/Lexer.cpp \
    "$PWD"/src/Parser/Parser.cpp \
    "$PWD"/src/Interpreter/Interpreter.cpp \
    "$PWD"/src/utils/utils.cpp \
    "$PWD"/main.cpp \
    -o "$PWD"/bin/build/interp/mac/vortex
}

winBuild()
{
    g++ \
    -O3 \
    -std=c++20 \
    -stdlib=libc++ \
    "$PWD"/src/Node/Node.cpp \
    "$PWD"/src/Lexer/Lexer.cpp \
    "$PWD"/src/Parser/Parser.cpp \
    "$PWD"/src/Interpreter/Interpreter.cpp \
    "$PWD"/src/utils/utils.cpp \
    "$PWD"/main.cpp \
    -o "$PWD"/bin/build/interp/windows/vortex.exe
}

linBuild()
{
    g++ \
    -O3 \
    -std=c++20 \
    -stdlib=libc++ \
    "$PWD"/src/Node/Node.cpp \
    "$PWD"/src/Lexer/Lexer.cpp \
    "$PWD"/src/Parser/Parser.cpp \
    "$PWD"/src/Interpreter/Interpreter.cpp \
    "$PWD"/src/utils/utils.cpp \
    "$PWD"/main.cpp \
    -o "$PWD"/bin/build/interp/windows/vortex.exe
}

OS="`uname`"
case $OS in
  'Linux')
    OS='Linux'
    echo "Building on Linux..."
    linBuild
    ;;
  'WindowsNT')
    OS='Windows'
    echo "Building on Windows..."
    winBuild
    ;;
  'Darwin') 
    OS='Mac'
    echo "Building on Mac..."
    macBuild
    ;;
  *) ;;
esac