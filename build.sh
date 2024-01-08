#!/bin/sh

macBuild()
{
    echo "Compiling Vortex..."
    mkdir "$PWD"/bin/build/interp
    mkdir "$PWD"/bin/build/interp/mac
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
    -o "$PWD"/bin/build/interp/mac/vortex  || { echo 'Compilation failed' ; exit 1; }

    while true; do
      read -p "Do you want to add Vortex to 'usr/local/bin' and built-in modules to 'usr/local/share'? - this will allow you to call Vortex globally and set it up with built-in modules [y/n] " yn
      case $yn in
          [Yy]* )
          echo "Password required to access usr/local"
          sudo -s eval 'mv "$PWD"/bin/build/interp/mac/vortex /usr/local/bin; sudo -s eval; mkdir /usr/local/share/vortex; mkdir /usr/local/share/vortex/modules; echo "Compiling modules..."; cd "Modules"; ./build_modules.sh; cd ..; cp -R "$PWD"/Modules/modules/* /usr/local/share/vortex/modules; cp "$PWD"/scripts/uninstall.sh /usr/local/share/vortex; chmod +x /usr/local/share/vortex/uninstall.sh'
          echo "Added Vortex and standard modules to usr/local - Important: To uninstall, you will need to run 'usr/local/share/vortex/uninstall.sh'"; break;;
          [Nn]* ) exit;;
          * ) echo "Please answer y or n.";;
      esac
    done
}

linBuild()
{
    echo "Compiling Vortex..."
    mkdir "$PWD"/bin/build/interp
    mkdir "$PWD"/bin/build/interp/linux
    clang++ \
    -Ofast \
    -Wno-everything \
    -std=c++20 \
    -stdlib=libc++ \
    -pthread \
    -ldl \
    "$PWD"/src/Node/Node.cpp \
    "$PWD"/src/Lexer/Lexer.cpp \
    "$PWD"/src/Parser/Parser.cpp \
    "$PWD"/src/Bytecode/Bytecode.cpp \
    "$PWD"/src/Bytecode/Generator.cpp \
    "$PWD"/src/VirtualMachine/VirtualMachine.cpp \
    "$PWD"/src/utils/utils.cpp \
    "$PWD"/main.cpp \
    -o "$PWD"/bin/build/interp/linux/vortex || { echo 'compilation failed' ; exit 1; }

    while true; do
      read -p "Do you want to add Vortex to 'usr/local/bin' and built-in modules to 'usr/local/share'? - this will allow you to call Vortex globally and set it up with built-in modules [y/n] " yn
      case $yn in
          [Yy]* )
          echo "Password required to access usr/local"
          sudo -s eval 'mv "$PWD"/bin/build/interp/linux/vortex /usr/local/bin; sudo -s eval; mkdir /usr/local/share/vortex; mkdir /usr/local/share/vortex/modules; echo "Compiling modules..."; cd "Modules"; ./build_modules_lin.sh; cd ..; cp -R "$PWD"/Modules/modules/* /usr/local/share/vortex/modules; cp "$PWD"/scripts/uninstall.sh /usr/local/share/vortex; chmod +x /usr/local/share/vortex/uninstall.sh'
          echo "Added Vortex and standard modules to usr/local - Important: To uninstall, you will need to run 'usr/local/share/vortex/uninstall.sh'"; break;;
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