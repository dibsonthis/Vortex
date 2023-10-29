#include <fstream>
#include <sstream>
#include <iostream>
#include "include/Vortex.hpp"

extern "C" Value open(std::vector<Value>& args) {
    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error("Function 'open' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value filePath = args[0];
    Value openMode = args[1];

    if (!filePath.is_string()) {
        error("Parameter 'filePath' must be a string");
    }

    if (!openMode.is_number()) {
        error("Parameter 'openMode' must be a number");
    }

    std::fstream* handle = new std::fstream(filePath.get_string(), (std::ios_base::openmode)(unsigned int)openMode.get_number());
    Value handlePtr = pointer_val();
    handlePtr.get_pointer()->value = std::move(handle);
    return handlePtr;
}

extern "C" Value read(std::vector<Value>& args) {
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error("Function 'read' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value fileHandle = args[0];

    if (!fileHandle.is_pointer()) {
        error("Parameter 'fileHandle' must be a pointer");
    }

    std::fstream* handle = (std::fstream*)fileHandle.get_pointer()->value;

    std::stringstream buffer;
    buffer << handle->rdbuf();
    handle->seekp(0);

    return string_val(buffer.str());
}

extern "C" Value write(std::vector<Value>& args) {
    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error("Function 'write' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value fileHandle = args[0];
    Value text = args[1];

    if (!fileHandle.is_pointer()) {
        error("Parameter 'fileHandle' must be a pointer");
    }

    if (!text.is_string()) {
        error("Parameter 'text' must be a string");
    }

    std::fstream* handle = (std::fstream*)fileHandle.get_pointer()->value;

    (*handle) << text.get_string();
    handle->flush();
    
    return none_val();
}

extern "C" Value close(std::vector<Value>& args) {
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error("Function 'close' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value fileHandle = args[0];

    if (!fileHandle.is_pointer()) {
        error("Parameter 'fileHandle' must be a pointer");
    }

    std::fstream* handle = (std::fstream*)fileHandle.get_pointer()->value;

    handle->flush();
    handle->close();
    delete handle;
    
    return none_val();
}

extern "C" Value input(std::vector<Value>& args) {
    int num_required_args = 0;

    if (args.size() != num_required_args) {
        error("Function 'input' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    std::string line;
    std::getline(std::cin, line);

    return string_val(line);
}