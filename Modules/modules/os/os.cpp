#include <stdlib.h>
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj get_env(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    VortexObj arg = args[0];

    if (arg->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    char* value = std::getenv(arg->_Node.String().value.c_str());

    std::string env_var = std::string(value == NULL ? "" : value);
    return new_string_node(env_var);
}

VortexObj set_env(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 2 argument");
    }

    VortexObj _name = args[0];
    VortexObj _value = args[1];

    if (_name->type != NodeType::STRING || _value->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 2 string arguments");
    }

    // std::string command = _name->_Node.String().value + "=" + _value->_Node.String().value;
    int res = setenv(_name->_Node.String().value.c_str(), _value->_Node.String().value.c_str(), 1);

    return new_number_node(res);
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "get_env") {
        return get_env(name, args);
    }
    if (name == "set_env") {
        return set_env(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}