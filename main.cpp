#include <iostream>
#include <fstream>
#include <filesystem>
#include "src/libs/JSON/json.hpp"
#include "src/Lexer/Lexer.hpp"
#include "src/Parser/Parser.hpp"
#include "src/Interpreter/Interpreter.hpp"

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
        std::filesystem::current_path("../../../source/playground");
        Lexer lexer("source.rpl");
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
            std::cout << "You must enter a source path e.g: ripple \"dev/main.rpl\"\n";
            return 1;
        }

        if (argc > 2)
        {
            std::cout << "Compiler only accepts 1 argument: source path";
            return 1;
        }

        std::string path = argv[1];

        Lexer lexer(path);
        lexer.tokenize();

        Parser parser(lexer.nodes, lexer.file_name);
        parser.parse(0, "_");
        parser.remove_op_node(";");
        auto ast = parser.nodes;

        auto parent_path = std::filesystem::path(path).parent_path();
        std::filesystem::current_path(parent_path);

        Interpreter interpreter(parser.nodes, parser.file_name);
        interpreter.evaluate();

        std::cin.get();
        exit(0);
    }
}