#include <fstream>
#include <sstream>
#include <iostream>
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj split(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 2 string arguments");
    }

    if (args[0]->type != NodeType::STRING || args[1]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 2 string arguments");
    }

    std::string& str = args[0]->_Node.String().value;
    std::string delim = args[1]->_Node.String().value;

    if (delim == "") {
        delim = " ";
    }

    size_t pos_start = 0, pos_end, delim_len = delim.length();
    std::string token;

    auto res = new_vortex_obj(NodeType::LIST);

    while ((pos_end = str.find(delim, pos_start)) != std::string::npos) {
        token = str.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;

        auto str_node = new_string_node(token);

        res->_Node.List().elements.push_back(str_node);
    }

    auto last_str_node = new_string_node(str.substr(pos_start));
    res->_Node.List().elements.push_back(last_str_node);

    return res;
}

VortexObj trim(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    if (args[0]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    VortexObj new_string = new_string_node(args[0]->_Node.String().value);

    new_string->_Node.String().value.erase(new_string->_Node.String().value.begin(), std::find_if(new_string->_Node.String().value.begin(), new_string->_Node.String().value.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));

    new_string->_Node.String().value.erase(std::find_if(new_string->_Node.String().value.rbegin(), new_string->_Node.String().value.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), new_string->_Node.String().value.end());

    return new_string;
}

VortexObj chars(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    if (args[0]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    node_ptr list = new_vortex_obj(NodeType::LIST);

    for (auto c : args[0]->_Node.String().value) {
        list->_Node.List().elements.push_back(new_string_node(std::string(1, c)));
    }

    return list;

}  

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "split") {
        return split(name, args);
    }
    if (name == "trim") {
        return trim(name, args);
    }
    if (name == "chars") {
        return chars(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}