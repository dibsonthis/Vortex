mkdir %CD%/bin/build/interp
mkdir %CD%/bin/build/interp/windows
g++ -O3 -std=c++20 %CD%/src/Node/Node.cpp %CD%/src/Lexer/Lexer.cpp %CD%/src/Parser/Parser.cpp %CD%/src/Interpreter/Interpreter.cpp %CD%/src/utils/utils.cpp %CD%/main.cpp -o %CD%/bin/build/interp/windows/vortex.exe