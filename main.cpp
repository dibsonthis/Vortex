#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include "src/libs/JSON/json.hpp"
#include "src/Lexer/Lexer.hpp"
#include "src/Parser/Parser.hpp"
#include "src/Interpreter/Interpreter.hpp"

enum Type
{
    DEV,
    INTERP,
};

Type type = Type::INTERP;

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

        Interpreter interpreter(parser.nodes, parser.file_name);
        interpreter.evaluate();

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

        Interpreter interpreter(parser.nodes, parser.file_name);
        interpreter.argc = argc-1;
        interpreter.argv = args;
        interpreter.evaluate();
        
        exit(0);
    }
}