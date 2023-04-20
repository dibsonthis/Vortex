#include <iostream>
#include <fstream>
#include <filesystem>
#include <dlfcn.h>
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

Type type = Type::DEV;

int main(int argc, char** argv)
{
    if (type == Type::DEV)
    {
        std::filesystem::current_path("../../../source/playground");

        // void* handle = dlopen("../libs/lib.dylib", RTLD_LAZY);
    
        // if (!handle) {
        //     std::cerr << "Cannot open library: " << dlerror() << '\n';
        //     return 1;
        // }

        // typedef void* (*load_t)();
        // typedef node_ptr (*call_function_t)(std::string name, void* handle, std::vector<node_ptr> args);

        // // reset errors
        // dlerror();

        // load_t load = (load_t) dlsym(handle, "load");
        // call_function_t call_function = (call_function_t) dlsym(handle, "call_function");

        // const char *dlsym_error = dlerror();
        // if (dlsym_error) {
        //     std::cerr << "Cannot load symbol 'hello': " << dlsym_error <<
        //         '\n';
        //     dlclose(handle);
        //     return 1;
        // }
        
        // // use it to do the calculation
        // void* cursorHandle = load();
        // node_ptr pos1 = call_function("get_cursor_pos", cursorHandle);
        // node_ptr pos2 = call_function("get_cursor_pos", cursorHandle);
        // std::cout << pos1->Object.properties["x"]->Number.value << ", " << pos1->Object.properties["x"]->Number.value << std::flush;
        // std::cout << "\n";
        // std::cout << pos2->Object.properties["x"]->Number.value << ", " << pos2->Object.properties["x"]->Number.value << std::flush;

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