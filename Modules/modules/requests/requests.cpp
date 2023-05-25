#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN
#include "../include/httplib.h"
#include "../../Vortex.hpp"
#include "../../../src/utils/utils.cpp"
#include "../../../src/Lexer/Lexer.cpp"
#include "../../../src/Parser/Parser.cpp"
#include "../../../src/Interpreter/Interpreter.cpp"

/* Define Vars */

Interpreter interp;
int indent_level = 0;
std::string indent = "    ";

/* Declare Lib Functions */

std::string to_string(VortexObj node, bool strip_quotes = false) {
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
            if (strip_quotes) {
                return node->_Node.String().value;
            }
            if (node->_Node.String().value.size() > 0 && node->_Node.String().value[0] == '"') {
                return node->_Node.String().value;
            }
            return '"' + node->_Node.String().value + '"';
        }
        case NodeType::LIST: {
            if (node->_Node.List().elements.size() == 0) {
                return "[]";
            }
            std::string res = "";
            res += "[\n";
            indent_level++;
            for (int i = 0; i < node->_Node.List().elements.size(); i++) {
                for (int ind = 0; ind < indent_level; ind++) {
                    res += indent;
                }
                res += to_string(node->_Node.List().elements[i]);
                if (i < node->_Node.List().elements.size()-1) {
                    res += ",\n";
                }
            }
            indent_level--;
            res += '\n';
            for (int i = 0; i < indent_level; i++) {
                res += indent;
            }
            res += "]";
            return res;
        }
        case NodeType::OBJECT: {
            if (node->_Node.Object().properties.size() == 0) {
                return "{}";
            }
            std::string res = "";
            res += "{\n";
            indent_level++;
            int elem_i = 0;
            for (auto const& elem : node->_Node.Object().properties) {
                for (int i = 0; i < indent_level; i++) {
                    res += indent;
                }
                res +=  '"' + elem.first + '"' + ": " + to_string(elem.second);
                if (elem_i != node->_Node.Object().properties.size()-1) {
                    res += ",\n";
                } else {
                    res += '\n';
                }
                elem_i++;
            }
            indent_level--;
            for (int i = 0; i < indent_level; i++) {
                res += indent;
            }
            res += "}";
            return res;
        }
        case NodeType::NONE: {
            return "null";
        }
        case NodeType::ID: {
            return '"' + node->_Node.ID().value + '"';
        }
        case NodeType::OP: {
            if (node->_Node.Op().value == ".") {
                return to_string(node->_Node.Op().left) + "." + to_string(node->_Node.Op().right);
            }
            return node->_Node.Op().value;
        }
        default: {
            return "";
        }
    }
}

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

VortexObj post(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 3) {
        error_and_exit("Function '" + name + "' expects 3 arguments");
    }

    if (args[0]->type != NodeType::STRING || args[1]->type != NodeType::STRING || (args[2]->type != NodeType::OBJECT && args[2]->type != NodeType::STRING)) {
        error_and_exit("Function '" + name + "' expects 2 string arguments and 1 object/string argument");
    }

    httplib::Client cli(args[0]->_Node.String().value);

    httplib::Result res;

    httplib::MultipartFormDataItems items;
    items.push_back({"data", args[2]->type == NodeType::STRING ? args[2]->_Node.String().value : to_string(args[2]), "", ""});
    res = cli.Post(args[1]->_Node.String().value, items);

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

VortexObj server(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 0;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    auto server = new httplib::Server();

    VortexObj server_ptr = new_vortex_obj(NodeType::POINTER);
    server_ptr->_Node.Pointer().value = server;
    return server_ptr;
}

VortexObj start(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 3;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_server = args[0];
    VortexObj v_host = args[1];
    VortexObj v_port = args[2];

    if (v_server->type != NodeType::POINTER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'server' must be a pointer";
        return error;
    }

    if (v_host->type != NodeType::STRING) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'host' must be a string";
        return error;
    }

    if (v_port->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'port' must be a number";
        return error;
    }

    httplib::Server* svr = (httplib::Server*)v_server->_Node.Pointer().value;

    svr->listen(v_host->_Node.String().value, v_port->_Node.Number().value);

    return new_vortex_obj(NodeType::NONE);
}

VortexObj set_get(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 4;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_server = args[0];
    VortexObj v_route = args[1];
    VortexObj v_callback = args[2];
    VortexObj v_content_type = args[3];

    if (v_server->type != NodeType::POINTER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'server' must be a pointer";
        return error;
    }

    if (v_route->type != NodeType::STRING) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'route' must be a string";
        return error;
    }

    if (v_route->_Node.String().value.size() == 0 || v_route->_Node.String().value[0] != '/') {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'route' must begin with '/'";
        return error;
    }

    if (v_callback->type != NodeType::FUNC) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'callback' must be a function";
        return error;
    }

    if (v_content_type->type != NodeType::STRING) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'content_type' must be a string";
        return error;
    }

    httplib::Server* svr = (httplib::Server*)v_server->_Node.Pointer().value;

    svr->Get(v_route->_Node.String().value, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type)](const httplib::Request &req, httplib::Response &res) {
        VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;
        VortexObj result = interp.eval_func_call(func_call, v_callback);

        std::string content_type = v_content_type->_Node.String().value;
        std::string result_string;
        if (content_type == "text/html") {
            result_string = to_string(result, true);
        } else {
            result_string = to_string(result);
        }
        res.set_content(result_string, content_type);
    });

    return new_vortex_obj(NodeType::NONE);
}

VortexObj set_post(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 4;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_server = args[0];
    VortexObj v_route = args[1];
    VortexObj v_callback = args[2];
    VortexObj v_content_type = args[3];

    if (v_server->type != NodeType::POINTER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'server' must be a pointer";
        return error;
    }

    if (v_route->type != NodeType::STRING) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'route' must be a string";
        return error;
    }

    if (v_route->_Node.String().value.size() == 0 || v_route->_Node.String().value[0] != '/') {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'route' must begin with '/'";
        return error;
    }

    if (v_callback->type != NodeType::FUNC) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'callback' must be a function";
        return error;
    }

    if (v_callback->_Node.Function().params.size() != 1) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'callback' must have one parameter";
        return error;
    }

    if (v_content_type->type != NodeType::STRING) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'content_type' must be a string";
        return error;
    }

    httplib::Server* svr = (httplib::Server*)v_server->_Node.Pointer().value;

    svr->Post(v_route->_Node.String().value, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type)](const httplib::Request &req, httplib::Response &res) {
        VortexObj req_object = new_vortex_obj(NodeType::OBJECT);

        if (req.params.size() > 0) {   
            for (auto& param : req.params) {
                req_object->_Node.Object().properties[param.first] = new_string_node(param.second);
            }
        } else if (req.files.size() > 0) {
            req_object = new_string_node(req.get_file_value("data").content);
        }

        VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().args.push_back(req_object);
        func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;

        VortexObj result = interp.eval_func_call(func_call, v_callback);

        std::string content_type = v_content_type->_Node.String().value;
        std::string result_string;

        if (content_type == "text/html") {
            result_string = to_string(result, true);
        } else {
            result_string = to_string(result);
        }

        res.set_content(result_string, content_type);
    });

    return new_vortex_obj(NodeType::NONE);
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "get") {
        return get(name, args);
    }
    if (name == "post") {
        return post(name, args);
    }
    if (name == "server") {
        return server(name, args);
    }
    if (name == "start") {
        return start(name, args);
    }
    if (name == "set_get") {
        return set_get(name, args);
    }
    if (name == "set_post") {
        return set_post(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}