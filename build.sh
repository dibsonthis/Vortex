#!/bin/sh

macBuild()
{
    mkdir "$PWD"/bin/build/interp
    mkdir "$PWD"/bin/build/interp/mac
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

linBuild()
{
    mkdir "$PWD"/bin/build/interp
    mkdir "$PWD"/bin/build/interp/linux
    clang++ \
    -O3 \
    -std=c++20 \
    "$PWD"/src/Node/Node.cpp \
    "$PWD"/src/Lexer/Lexer.cpp \
    "$PWD"/src/Parser/Parser.cpp \
    "$PWD"/src/Interpreter/Interpreter.cpp \
    "$PWD"/src/utils/utils.cpp \
    "$PWD"/main.cpp \
    -o "$PWD"/bin/build/interp/linux/vortex
}

OS="`uname`"
case $OS in
  'Linux')
    OS='Linux'
    echo "Building on Linux..."
    linBuild
    ;;
  'Darwin') 
    OS='Mac'
    echo "Building on Mac..."
    macBuild
    ;;
  *) ;;
esac