#!/bin/bash

build()
{
    echo "Compiling Vortex..."
    mkdir "$PWD"/bin/build
    mkdir "$PWD"/bin/build/interp
    mkdir "$PWD"/bin/build/interp/win

    clang++ \
    -Ofast \
    -Wno-everything \
    -std=c++20 \
    "$PWD"/src/Node/Node.cpp \
    "$PWD"/src/Lexer/Lexer.cpp \
    "$PWD"/src/Parser/Parser.cpp \
    "$PWD"/src/Bytecode/Bytecode.cpp \
    "$PWD"/src/Bytecode/Generator.cpp \
    "$PWD"/src/VirtualMachine/VirtualMachine.cpp \
    "$PWD"/src/utils/utils.cpp \
    "$PWD"/main.cpp \
    -o "$PWD"/bin/build/interp/win/vortex || { echo 'compilation failed' ; $SHELL; exit 1; }

    while true; do
      read -p "Do you want to add Vortex to 'C:\Program Files' and built-in modules to C:\Windows\System32\vortex\modules'? - this will allow you to call Vortex globally and set it up with built-in modules [y/n] " yn
      case $yn in
          [Yy]* )
          mv "$PWD"/bin/build/interp/win/vortex.exe "C:/Program Files"; 
          mkdir "C:/Program Files/vortex"; 
          mkdir "C:/Program Files/vortex/modules"; 
          echo "Compiling modules..."; 
          cd "Modules"; 
          ./build_modules_win.sh; 
          cd ..; 
          cp -r "$PWD"/Modules/modules/* "C:/Program Files/vortex/modules"; 
          cp "$PWD"/scripts/uninstall.sh "C:/Program Files/vortex";
          echo "Added Vortex and standard modules to C:Program Files\vortex - Important: To uninstall, you will need to run uninstall.sh'"; break;;
          [Nn]* ) exit;;
          * ) echo "Please answer y or n.";;
      esac
    done
}

build

$SHELL