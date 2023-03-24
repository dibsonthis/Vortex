#include <iostream>
#include <fstream>
#include "src/Lexer/Lexer.hpp"
#include "src/Parser/Parser.hpp"

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
        Lexer lexer("../../../source.txt");
        lexer.tokenize();

        Parser parser(lexer.nodes);
        parser.parse(0, "_");
        auto ast = parser.nodes;

        std::cin.get();
        exit(0);
    }

    if (type == Type::INTERP)
    {
        if (argc == 1)
        {
            std::cout << "You must enter a source path e.g: glide \"main.gl\"\n";
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

        exit(0);
    }
}