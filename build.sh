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

    while true; do
      read -p "Do you want to add Vortex to 'usr/local/bin'? " yn
      case $yn in
          [Yy]* ) sudo mv "$PWD"/bin/build/interp/mac/vortex /usr/local/bin; echo "Added Vortex to usr/local/bin - Important: To uninstall, you'll need to run 'rm /usr/local/bin/vortex'"; break;;
          [Nn]* ) exit;;
          * ) echo "Please answer y or n.";;
      esac
    done
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

    while true; do
      read -p "Do you want to add Vortex to 'usr/local/bin'? " yn
      case $yn in
          [Yy]* ) sudo mv "$PWD"/bin/build/interp/linux/vortex /usr/local/bin; echo "Added Vortex to usr/local/bin - Important: To uninstall, you'll need to run 'rm /usr/local/bin/vortex'"; break;;
          [Nn]* ) exit;;
          * ) echo "Please answer y or n.";;
      esac
    done
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