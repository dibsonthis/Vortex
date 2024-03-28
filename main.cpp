#include <iostream>
#include <fstream>
#include <filesystem>
#include "src/Lexer/Lexer.hpp"
#include "src/Parser/Parser.hpp"
#include "src/Bytecode/Bytecode.hpp"
#include "src/Bytecode/Generator.hpp"
#include "src/VirtualMachine/VirtualMachine.hpp"

enum CompType
{
    DEV,
    INTERP,
};

CompType type = CompType::INTERP;

int main(int argc, char **argv)
{
    if (type == CompType::DEV)
    {
        // std::filesystem::current_path("../../../playground/gui/src");
        // Lexer lexer("main.vtx");
        std::filesystem::current_path("../../../playground");
        Lexer lexer("source.vtx");

        lexer.tokenize();

        Parser parser(lexer.nodes, lexer.file_name);
        parser.parse(0, "_");
        parser.remove_op_node(";");
        auto ast = parser.nodes;

        VM vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.name = "source.vtx";
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;

        generate_bytecode(parser.nodes, main_frame.function->chunk, parser.file_name);
        add_code(main_frame.function->chunk, OP_EXIT);
        disassemble_chunk(main_frame.function->chunk, "Test");
        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;
        vm.frames.push_back(main_frame);
        evaluate(vm);

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
        std::string import_path;

        if (argc > 1)
        {
            for (int i = 1; i < argc; i++)
            {
                args.push_back(argv[i]);
            }
        }

        for (int i = 0; i < args.size(); i++)
        {
            std::string arg = args[i];
            if (arg == "-m" || arg == "-modules")
            {
                if (i == args.size() - 1)
                {
                    std::cout << "Invalid module path";
                    return 1;
                }
                else
                {
                    import_path = args[i + 1];
                }
            }
        }

        Lexer lexer(path);
        lexer.tokenize();

        auto parent_path = std::filesystem::path(path).parent_path();
        if (parent_path != "")
        {
            std::filesystem::current_path(parent_path);
        }

        Parser parser(lexer.nodes, lexer.file_name);
        parser.parse(0, "_");
        parser.remove_op_node(";");
        auto ast = parser.nodes;

        VM vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        main->chunk.import_path = import_path;
        CallFrame main_frame;
        main_frame.name = path;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;

        generate_bytecode(parser.nodes, main_frame.function->chunk, path);
        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;
        vm.frames.push_back(main_frame);
        add_code(main_frame.function->chunk, OP_EXIT);
        evaluate(vm);

        exit(0);
    }
}