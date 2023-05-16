#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN
#include "../include/httplib.h"
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj get(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 2 arguments");
    }

    if (args[0]->type != NodeType::STRING || args[1]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 2 string arguments");
    }

    httplib::Client cli(args[0]->_Node.String().value);

    auto res = cli.Get(args[1]->_Node.String().value);

    VortexObj response = new_vortex_obj(NodeType::OBJECT);

    if (!res) {
        response->_Node.Object().properties["version"] = new_string_node("");
        response->_Node.Object().properties["status"] = new_number_node(-1);
        response->_Node.Object().properties["body"] = new_string_node("");
        response->_Node.Object().properties["location"] = new_string_node("");
        response->_Node.Object().properties["headers"] = new_vortex_obj(NodeType::LIST);
        return response;
    }

    response->_Node.Object().properties["version"] = new_string_node(res->version);
    response->_Node.Object().properties["status"] = new_number_node(res->status);
    response->_Node.Object().properties["body"] = new_string_node(res->body);
    response->_Node.Object().properties["location"] = new_string_node(res->location);
    response->_Node.Object().properties["headers"] = new_vortex_obj(NodeType::LIST);
    for (auto& elem : res->headers) {
        response->_Node.Object().properties["headers"]->_Node.List().elements.push_back(new_string_node(elem.first));
    }

    return response;
}

std::string to_string(VortexObj node) {
    switch (node->type) {
        case NodeType::NUMBER: {
            std::string num_str = std::to_string(node->_Node.Number().value);
            num_str.erase(num_str.find_last_not_of('0') + 1, std::string::npos);
            num_str.erase(num_str.find_last_not_of('.') + 1, std::string::npos);
            return num_str;
        }
        case NodeType::BOOLEAN: {
            return node->_Node.Boolean().value ? "true" : "false";
        }
        case NodeType::STRING: {
            return node->_Node.String().value;
        }
        case NodeType::FUNC: {
            std::string res = "(";
            for (int i = 0; i < node->_Node.Function().params.size(); i++) {
                node_ptr& param = node->_Node.Function().params[i];
                node_ptr& type = node->_Node.Function().param_types[param->_Node.ID().value];
                if (type) {
                    res += param->_Node.ID().value + ": " + node_repr(type);
                } else {
                    res += param->_Node.ID().value;
                }
                if (i < node->_Node.Function().params.size()-1) {
                    res += ", ";
                }
            }
            if (node->_Node.Function().return_type) {
                res += ") => " + node_repr(node->_Node.Function().return_type);
            } else {
                res += ") => ...";
            }
            if (node->_Node.Function().dispatch_functions.size() > 0) {
                res += "\n";
                for (auto& func : node->_Node.Function().dispatch_functions) {
                    res += to_string(func) += "\n";
                }
            }
            return res;
        }
        case NodeType::LIST: {
            std::string res = "[";
            for (int i = 0; i < node->_Node.List().elements.size(); i++) {
                res += to_string(node->_Node.List().elements[i]);
                if (i < node->_Node.List().elements.size()-1) {
                    res += ", ";
                }
            }
            res += "]";
            return res;
        }
        case NodeType::OBJECT: {
            std::string res = "{ ";
            for (auto const& elem : node->_Node.Object().properties) {
                res += elem.first + ": " + to_string(elem.second) + ' ';
            }
            res += "}";
            return res;
        }
        case NodeType::POINTER: {
            std::stringstream buffer;
            buffer << node->_Node.Pointer().value;
            return buffer.str();
        }
        case NodeType::LIB: {
            std::stringstream buffer;
            buffer << node->_Node.Lib().handle;
            return buffer.str();
        }
        case NodeType::NONE: {
            return "None";
        }
        case NodeType::ID: {
            return node->_Node.ID().value;
        }
        case NodeType::OP: {
            if (node->_Node.Op().value == ".") {
                return to_string(node->_Node.Op().left) + "." + to_string(node->_Node.Op().right);
            }
            return node->_Node.Op().value;
        }
        case NodeType::ACCESSOR: {
            return to_string(node->_Node.Accessor().container) + to_string(node->_Node.Accessor().accessor);
        }
        default: {
            return "<not implemented>";
        }
    }
}

VortexObj post(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 3) {
        error_and_exit("Function '" + name + "' expects 3 arguments");
    }

    if (args[0]->type != NodeType::STRING || args[1]->type != NodeType::STRING || args[2]->type != NodeType::OBJECT) {
        error_and_exit("Function '" + name + "' expects 2 string arguments and 1 object argument");
    }

    httplib::Client cli(args[0]->_Node.String().value);

    httplib::Params params;

    for (auto& prop : args[2]->_Node.Object().properties) {
        params.emplace(prop.first, to_string(prop.second));
    }

    auto res = cli.Post(args[1]->_Node.String().value, params);

    VortexObj response = new_vortex_obj(NodeType::OBJECT);

    if (!res) {
        response->_Node.Object().properties["version"] = new_string_node("");
        response->_Node.Object().properties["status"] = new_number_node(-1);
        response->_Node.Object().properties["body"] = new_string_node("");
        response->_Node.Object().properties["location"] = new_string_node("");
        response->_Node.Object().properties["headers"] = new_vortex_obj(NodeType::LIST);
        return response;
    }

    response->_Node.Object().properties["version"] = new_string_node(res->version);
    response->_Node.Object().properties["status"] = new_number_node(res->status);
    response->_Node.Object().properties["body"] = new_string_node(res->body);
    response->_Node.Object().properties["location"] = new_string_node(res->location);
    response->_Node.Object().properties["headers"] = new_vortex_obj(NodeType::LIST);
    for (auto& elem : res->headers) {
        response->_Node.Object().properties["headers"]->_Node.List().elements.push_back(new_string_node(elem.first));
    }

    return response;
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "get") {
        return get(name, args);
    }
    if (name == "post") {
        return post(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}