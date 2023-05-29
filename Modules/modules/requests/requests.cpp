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

    httplib::MultipartFormDataItems items;
    items.push_back({"data", args[2]->type == NodeType::STRING ? args[2]->_Node.String().value : to_string(args[2]), "", ""});
    auto res = cli.Post(args[1]->_Node.String().value, items);

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

    server->set_exception_handler([](const auto& req, auto& res, std::exception_ptr ep) {
        auto fmt = "<h1>Error 500</h1><p>%s</p>";
        char buf[BUFSIZ];
        try {
            std::rethrow_exception(ep);
        } catch (std::exception &e) {
            snprintf(buf, sizeof(buf), fmt, e.what());
        } catch (...) { // See the following NOTE
            snprintf(buf, sizeof(buf), fmt, "Unknown Exception");
        }
        res.set_content(buf, "text/html");
        res.status = 500;
    });

    VortexObj server_ptr = new_vortex_obj(NodeType::POINTER);
    server_ptr->_Node.Pointer().value = server;
    return server_ptr;
}

VortexObj server_ssl(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj cert_path = args[0];
    VortexObj private_key_path = args[1];

    if (cert_path->type != NodeType::STRING) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'cert_path' must be a string";
        return error;
    }

    if (private_key_path->type != NodeType::STRING) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'private_key_path' must be a string";
        return error;
    }

    auto server = new httplib::SSLServer(cert_path->_Node.String().value.c_str(), private_key_path->_Node.String().value.c_str());

    server->set_exception_handler([](const auto& req, auto& res, std::exception_ptr ep) {
        auto fmt = "<h1>Error 500</h1><p>%s</p>";
        char buf[BUFSIZ];
        try {
            std::rethrow_exception(ep);
        } catch (std::exception &e) {
            snprintf(buf, sizeof(buf), fmt, e.what());
        } catch (...) { // See the following NOTE
            snprintf(buf, sizeof(buf), fmt, "Unknown Exception");
        }
        res.set_content(buf, "text/html");
        res.status = 500;
    });

    VortexObj server_ptr = new_vortex_obj(NodeType::POINTER);
    server_ptr->_Node.Pointer().value = server;
    return server_ptr;
}

VortexObj start(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 4;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_server = args[0];
    VortexObj v_host = args[1];
    VortexObj v_port = args[2];
    VortexObj v_ssl = args[3];

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

    if (v_ssl->type != NodeType::BOOLEAN) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'ssl' must be a boolean";
        return error;
    }

    if (v_ssl->_Node.Boolean().value == true) {
        std::cout << "HERE";
    }

    if (v_ssl->_Node.Boolean().value) {
        httplib::SSLServer* svr = (httplib::SSLServer*)v_server->_Node.Pointer().value;
        svr->listen(v_host->_Node.String().value, v_port->_Node.Number().value);
        return new_vortex_obj(NodeType::NONE);
    }

    httplib::Server* svr = (httplib::Server*)v_server->_Node.Pointer().value;
    svr->listen(v_host->_Node.String().value, v_port->_Node.Number().value);
    return new_vortex_obj(NodeType::NONE);
}

std::string parse_route(std::string& route, std::vector<std::string>& params) {

    // We want to parse the route in case we have parameters
    // If we do, we store the param names and replace the param with
    // the following regex: ([a-zA-Z0-9_-]+)
    // If we have matches, we insert the matches into the callback closure
    // With the correct names we pulled out earlier

    std::string parsed_route;
    std::string regex = "([a-zA-Z0-9_-]+)";

    for (int i = 0; i < route.size(); i++) {
        if (route[i] == '{') {
            std::string param;
            i++;
            parsed_route += regex;
            while (route[i] != '}') {
                if (route[i] == '{' || !(isalpha(route[i]) || route[i] == '_')) {
                    // No nesting allowed
                    return "";
                }
                if (i == route.size()) {
                    return "";
                }
                param += route[i];
                i++;
            }
            // Bypass '}'
            i++;
            params.push_back(param);
        }

        if (i < route.size()) {
            parsed_route += route[i];
        }
    }

    return parsed_route;
}

VortexObj set_get(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 5;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_server = args[0];
    VortexObj v_route = args[1];
    VortexObj v_callback = args[2];
    VortexObj v_content_type = args[3];
    VortexObj v_ssl = args[4];

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

    if (v_ssl->type != NodeType::BOOLEAN) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'ssl' must be a boolean";
        return error;
    }

    std::vector<std::string> params;
    auto route = v_route->_Node.String().value;
    auto parsed_route = parse_route(route, params);

    if (v_ssl->_Node.Boolean().value == true) {
        std::cout << "TRUE" << std::endl;
    }

    if (v_ssl->_Node.Boolean().value) {
        httplib::SSLServer* svr = (httplib::SSLServer*)v_server->_Node.Pointer().value;

        svr->Get(parsed_route, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type), params = std::move(params)](const httplib::Request &req, httplib::Response &res) {
            for (int i = 0; i < params.size(); i++) {
                auto param = req.matches[i+1];
                v_callback->_Node.Function().closure[params[i]] = new_string_node(param);
            }
            VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
            func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;
            VortexObj result = interp.eval_func_call(func_call, v_callback);
            
            if (result->type == NodeType::ERROR) {
                throw std::runtime_error(result->_Node.Error().message);
            }

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

    httplib::Server* svr = (httplib::Server*)v_server->_Node.Pointer().value;

    svr->Get(parsed_route, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type), params = std::move(params)](const httplib::Request &req, httplib::Response &res) {
        for (int i = 0; i < params.size(); i++) {
            auto param = req.matches[i+1];
            v_callback->_Node.Function().closure[params[i]] = new_string_node(param);
        }
        VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;
        VortexObj result = interp.eval_func_call(func_call, v_callback);
        
        if (result->type == NodeType::ERROR) {
            throw std::runtime_error(result->_Node.Error().message);
        }

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

    int num_required_args = 5;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_server = args[0];
    VortexObj v_route = args[1];
    VortexObj v_callback = args[2];
    VortexObj v_content_type = args[3];
    VortexObj v_ssl = args[4];

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

    if (v_ssl->type != NodeType::BOOLEAN) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'ssl' must be a boolean";
        return error;
    }

    std::vector<std::string> params;
    auto route = v_route->_Node.String().value;
    auto parsed_route = parse_route(route, params);

    if (v_ssl->_Node.Boolean().value) {
        httplib::SSLServer* svr = (httplib::SSLServer*)v_server->_Node.Pointer().value;

        svr->Post(parsed_route, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type), params = std::move(params)](const httplib::Request &req, httplib::Response &res) {
            VortexObj req_object = new_vortex_obj(NodeType::OBJECT);

            if (req.params.size() > 0) {   
                for (auto& param : req.params) {
                    req_object->_Node.Object().properties[param.first] = new_string_node(param.second);
                }
            } else if (req.files.size() > 0) {
                req_object = new_string_node(req.get_file_value("data").content);
            }

            for (int i = 0; i < params.size(); i++) {
                auto param = req.matches[i+1];
                v_callback->_Node.Function().closure[params[i]] = new_string_node(param);
            }

            VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
            func_call->_Node.FunctionCall().args.push_back(req_object);
            func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;

            VortexObj result = interp.eval_func_call(func_call, v_callback);

            if (result->type == NodeType::ERROR) {
                throw std::runtime_error(result->_Node.Error().message);
            }

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

    httplib::Server* svr = (httplib::Server*)v_server->_Node.Pointer().value;

    svr->Post(parsed_route, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type), params = std::move(params)](const httplib::Request &req, httplib::Response &res) {
        VortexObj req_object = new_vortex_obj(NodeType::OBJECT);

        if (req.params.size() > 0) {   
            for (auto& param : req.params) {
                req_object->_Node.Object().properties[param.first] = new_string_node(param.second);
            }
        } else if (req.files.size() > 0) {
            req_object = new_string_node(req.get_file_value("data").content);
        }

        for (int i = 0; i < params.size(); i++) {
            auto param = req.matches[i+1];
            v_callback->_Node.Function().closure[params[i]] = new_string_node(param);
        }

        VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().args.push_back(req_object);
        func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;

        VortexObj result = interp.eval_func_call(func_call, v_callback);

        if (result->type == NodeType::ERROR) {
            throw std::runtime_error(result->_Node.Error().message);
        }

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
    if (name == "server_ssl") {
        return server_ssl(name, args);
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