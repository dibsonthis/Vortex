#include "../../Vortex.hpp"
#include "../../../src/utils/utils.cpp"
#include "../../../src/Lexer/Lexer.cpp"
#include "../../../src/Parser/Parser.cpp"
#include "../../../src/Interpreter/Interpreter.cpp"

/* Define Vars */

Interpreter _interpreter = Interpreter();
std::reference_wrapper<Interpreter> _interpreter_ref = std::ref(_interpreter);

/* Implement Lib Functions */

VortexObj pause(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 argument");
    }

    std::cin.get();

    return new_vortex_obj(NodeType::NONE);
}

VortexObj argc(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 argument");
    }

    return new_number_node(_interpreter_ref.get().argc);
}

VortexObj argv(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 argument");
    }

    VortexObj argv = new_vortex_obj(NodeType::LIST);

    for (std::string& arg : _interpreter_ref.get().argv) {
        argv->_Node.List().elements.push_back(new_string_node(arg));
    }

    return argv;
}

/* Implement load [Optional] */

extern "C" void load(Interpreter& interpreter) {
   _interpreter_ref = std::ref(interpreter);
   _interpreter = _interpreter_ref.get();
   
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "pause") {
        return pause(name, args);
    }
    if (name == "argc") {
        return argc(name, args);
    }
    if (name == "argv") {
        return argv(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}