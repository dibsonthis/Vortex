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

enum CompType
{
    DEV,
    INTERP,
};

CompType type = CompType::DEV;

int main(int argc, char **argv)
{
    if (type == CompType::DEV)
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
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.name = "source";
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;

        generate_bytecode(parser.nodes, main_frame.function->chunk);
        add_code(main_frame.function->chunk, OP_EXIT);
        disassemble_chunk(main_frame.function->chunk, "Test");
        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;
        vm.frames.push_back(main_frame);
        evaluate(vm);

        // Interpreter interpreter(typechecker.nodes, parser.file_name);
        // interpreter.evaluate();

        std::cin.get();
        exit(0);
    }

    if (type == CompType::INTERP)
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
            for (int i = 1; i < argc; i++)
            {
                args.push_back(argv[i]);
            }
        }

        Lexer lexer(path);
        lexer.tokenize();

        // Parser parser(lexer.nodes, lexer.file_name);
        // parser.parse(0, "_");
        // parser.remove_op_node(";");
        // auto ast = parser.nodes;

        auto parent_path = std::filesystem::path(path).parent_path();
        if (parent_path != "")
        {
            std::filesystem::current_path(parent_path);
        }

        // Typechecker typechecker(parser.nodes, parser.file_name);
        // typechecker.typecheck();

        // Interpreter interpreter(typechecker.nodes, parser.file_name);
        // interpreter.evaluate();

        // exit(0);

        Parser parser(lexer.nodes, lexer.file_name);
        parser.parse(0, "_");
        parser.remove_op_node(";");
        auto ast = parser.nodes;

        // Typechecker typechecker(parser.nodes, parser.file_name);
        // typechecker.typecheck();

        VM vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.name = path;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;

        generate_bytecode(parser.nodes, main_frame.function->chunk);
        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;
        vm.frames.push_back(main_frame);
        add_code(main_frame.function->chunk, OP_EXIT);
        evaluate(vm);

        exit(0);
    }
}