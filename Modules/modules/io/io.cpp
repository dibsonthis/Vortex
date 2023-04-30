#include <fstream>
#include <sstream>
#include <iostream>
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj read(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1 || args[0]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    std::ifstream t(args[0]->String.value);
    std::stringstream buffer;
    buffer << t.rdbuf();

    return new_string_node(buffer.str());
}

VortexObj write(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2 || args[0]->type != NodeType::STRING || args[1]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 2 string arguments");
    }

    std::string file_path = args[0]->String.value;
    std::string content = args[1]->String.value;

    std::ofstream open_file(file_path);
    open_file << content;
    open_file.close();

    return new_vortex_obj(NodeType::NONE);
}

VortexObj append(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2 || args[0]->type != NodeType::STRING || args[1]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 2 string arguments");
    }

    std::string file_path = args[0]->String.value;
    std::string content = args[1]->String.value;

    std::ofstream open_file(file_path, std::ofstream::app);
    open_file << content;
    open_file.close();

    return new_vortex_obj(NodeType::NONE);
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "read") {
        return read(name, args);
    }
    if (name == "write") {
        return write(name, args);
    }
    if (name == "append") {
        return append(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}