#include <iostream>
#include <fstream>
#include <filesystem>
#include "src/Lexer/Lexer.hpp"
#include "src/Parser/Parser.hpp"
#include "src/Interpreter/Interpreter.hpp"
#include "src/Typechecker/Typechecker.hpp"
#include "src/Bytecode/Bytecode.hpp"
#include "src/Bytecode/Generator.hpp"
#include "src/VirtualMachine/VirtualMachine.hpp"

enum Type
{
    DEV,
    INTERP,
};

Type type = Type::DEV;

int main(int argc, char** argv)
{
    if (type == Type::DEV)
    {
        std::filesystem::current_path("../../../playground");
        Lexer lexer("source.vtx");

        lexer.tokenize();

        Parser parser(lexer.nodes, lexer.file_name);
        parser.parse(0, "_");
        parser.remove_op_node(";");
        auto ast = parser.nodes;

        // Typechecker typechecker(parser.nodes, parser.file_name);
        // typechecker.typecheck();

        VM vm;
        Chunk chunk;

        generate_bytecode(parser.nodes, chunk);
        disassemble_chunk(chunk, "Test");
        evaluate(vm, chunk);

        // Interpreter interpreter(typechecker.nodes, parser.file_name);
        // interpreter.evaluate();

        std::cin.get();
        exit(0);
    }

    if (type == Type::INTERP)
    {
        if (argc == 1)
        {
            std::cout << "You must enter a source path e.g: vortex \"dev/main.vtx\"\n";
            return 1;
        }

        std::string path = argv[1];
        std::vector<std::string> args;

        if (argc > 1)
        {
            for (int i = 1; i < argc; i++) {
                args.push_back(argv[i]);
            }
        }

        Lexer lexer(path);
        lexer.tokenize();

        Parser parser(lexer.nodes, lexer.file_name);
        parser.parse(0, "_");
        parser.remove_op_node(";");
        auto ast = parser.nodes;

        auto parent_path = std::filesystem::path(path).parent_path();
        if (parent_path != "") {
            std::filesystem::current_path(parent_path);
        }

        Typechecker typechecker(parser.nodes, parser.file_name);
        typechecker.typecheck();

        Interpreter interpreter(typechecker.nodes, parser.file_name);
        interpreter.evaluate();
        
        exit(0);
    }
}