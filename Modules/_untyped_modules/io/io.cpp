#include <fstream>
#include <sstream>
#include <iostream>
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj open(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2 || args[0]->type != NodeType::STRING || args[1]->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 2 string arguments");
    }

    std::fstream* handle = new std::fstream(args[0]->_Node.String().value, (uint)args[1]->_Node.Number().value);
    VortexObj handle_ptr = new_vortex_obj(NodeType::POINTER);
    handle_ptr->_Node.Pointer().value = std::move(handle);
    return handle_ptr;
}

VortexObj read(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1 || args[0]->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects 1 pointer argument");
    }

    std::fstream* handle = (std::fstream*)args[0]->_Node.Pointer().value;
    std::stringstream buffer;
    buffer << handle->rdbuf();
    handle->seekp(0);

    return new_string_node(buffer.str());
}

VortexObj write(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2 || args[0]->type != NodeType::POINTER || args[1]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 pointer and 1 string arguments");
    }

    std::fstream* handle = (std::fstream*)args[0]->_Node.Pointer().value;
    std::string content = args[1]->_Node.String().value;

    (*handle) << content;
    handle->flush();
    
    return new_vortex_obj(NodeType::NONE);
}

VortexObj close(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1 || args[0]->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects 1 pointer argument");
    }

    std::fstream* handle = (std::fstream*)args[0]->_Node.Pointer().value;
    handle->flush();
    handle->close();
    delete handle;
    return new_vortex_obj(NodeType::NONE);
}

VortexObj input(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }

    std::string line;
    std::getline(std::cin, line);

    return new_string_node(line);
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "open") {
        return open(name, args);
    }
    if (name == "close") {
        return close(name, args);
    }
    if (name == "read") {
        return read(name, args);
    }
    if (name == "write") {
        return write(name, args);
    }
    if (name == "input") {
        return input(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}