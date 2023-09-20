#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN
#include "include/Vortex.hpp"
#include "include/httplib.h"

std::string toString(Value value, bool quote_strings = false) {
    switch(value.type) {
        case ValueType::Number: {
            double number = value.get_number();
            if (std::floor(number) == number) {
                return std::to_string((long long)number);
            }
            char buff[100];
            snprintf(buff, sizeof(buff), "%.8g", number);
            std::string buffAsStdStr = buff;
            return buffAsStdStr;
        }
        case ValueType::String: {
            if (quote_strings) {
                return "\"" + value.get_string() + "\"";
            }
            return(value.get_string());
        }
        case ValueType::Boolean: {
            return (value.get_boolean() ? "true" : "false");
        }
        case ValueType::List: {
            std::string repr = "[";
            for (int i = 0; i < value.get_list()->size(); i++) {
                Value& v = value.get_list()->at(i);
                repr += toString(v, quote_strings);
                if (i < value.get_list()->size()-1) {
                    repr += ", ";
                }
            }
            repr += "]";
            return repr;
        }
        case ValueType::Type: {
            return "Type: " + value.get_type()->name;
        }
        case ValueType::Object: {
            auto& obj = value.get_object();
            std::string repr;
            if (obj->type) {
                repr += value.get_object()->type->name + " ";
            }
            repr += "{ ";
            int i = 0;
            int size = obj->values.size();
            for (auto& prop : obj->values) {
                std::string key = prop.first;
                if (quote_strings) {
                    key = "\"" + key + "\"";
                }
                repr += key + ": " + toString(prop.second, quote_strings);
                i++;
                if (i < size) {
                    repr += ", ";
                }
            }
            repr += " }";
            return repr;
        }
        case ValueType::Function: {
            return "Function: " + value.get_function()->name;
        }
        case ValueType::Native: {
            return "<Native Function>";
        }
        case ValueType::Pointer: {
            return "<Pointer>";
        }
        case ValueType::None: {
            return "None";
        }
        default: {
            return "Undefined";
        }
    }
}

extern "C" Value _get(std::vector<Value>& args) {
    int num_required_args = 3;

    if (args.size() != num_required_args) {
        error("Function 'get' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value url = args[0];
    Value endpoint = args[1];
    Value headers = args[2];

    if (!url.is_string()) {
        error("Function 'get' expects argument 'url' to be a string");
    }

    if (!endpoint.is_string()) {
        error("Function 'get' expects argument 'endpoint' to be a string");
    }

    if (!headers.is_object()) {
        error("Function 'get' expects argument 'headers' to be a object");
    }

    httplib::Headers _headers;
    for (auto& prop : headers.get_object()->values) {
        _headers.insert({prop.first, toString(prop.second)});
    }

    httplib::Client cli(url.get_string());
    cli.enable_server_certificate_verification(false);

    auto res = cli.Get(endpoint.get_string(), _headers);

    Value response = object_val();

    if (!res) {
        response.get_object()->values["version"] = string_val("");
        response.get_object()->values["status"] = number_val(-1);
        response.get_object()->values["body"] = string_val("");
        response.get_object()->values["location"] = string_val("");
        response.get_object()->values["headers"] = list_val();
        return response;
    }

    response.get_object()->values["version"] = string_val(res->version);
    response.get_object()->values["status"] = number_val(res->status);
    response.get_object()->values["body"] = string_val(res->body);
    response.get_object()->values["location"] = string_val(res->location);
    response.get_object()->values["headers"] = list_val();
    for (auto& elem : res->headers) {
        response.get_object()->values["headers"].get_list()->push_back(string_val(elem.first));
    }

    return response;
}

extern "C" Value _post(std::vector<Value>& args) {
    int num_required_args = 4;

    if (args.size() != num_required_args) {
        error("Function 'post' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value url = args[0];
    Value endpoint = args[1];
    Value payload = args[2];
    Value headers = args[3];

    if (!url.is_string()) {
        error("Function 'post' expects argument 'url' to be a string");
    }

    if (!endpoint.is_string()) {
        error("Function 'post' expects argument 'endpoint' to be a string");
    }

    if (!payload.is_object() && !payload.is_string()) {
        error("Function 'post' expects argument 'payload' to be an object or string");
    }

    if (!headers.is_object()) {
        error("Function 'post' expects argument 'headers' to be an object");
    }

    httplib::Client cli(url.get_string());
    cli.enable_server_certificate_verification(false);

    httplib::Headers _headers;
    for (auto& prop : headers.get_object()->values) {
        _headers.insert({prop.first, toString(prop.second)});
    }

    httplib::Result res = httplib::Result(nullptr, httplib::Error());

    if (payload.is_object()) {
        httplib::MultipartFormDataItems items;
        for (auto& prop : payload.get_object()->values) {
            items.push_back({prop.first, toString(prop.second), "", ""});
        }
        res = cli.Post(endpoint.get_string(), _headers, items);
    } else {
        res = cli.Post(endpoint.get_string(), _headers, payload.get_string(), "");
    }

    Value response = object_val();

    if (!res) {
        response.get_object()->values["version"] = string_val("");
        response.get_object()->values["status"] = number_val(-1);
        response.get_object()->values["body"] = string_val("");
        response.get_object()->values["location"] = string_val("");
        response.get_object()->values["headers"] = list_val();
        return response;
    }

    response.get_object()->values["version"] = string_val(res->version);
    response.get_object()->values["status"] = number_val(res->status);
    response.get_object()->values["body"] = string_val(res->body);
    response.get_object()->values["location"] = string_val(res->location);
    response.get_object()->values["headers"] = list_val();
    for (auto& elem : res->headers) {
        response.get_object()->values["headers"].get_list()->push_back(string_val(elem.first));
    }

    return response;
}

extern "C" Value _put(std::vector<Value>& args) {
    int num_required_args = 4;

    if (args.size() != num_required_args) {
        error("Function 'put' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value url = args[0];
    Value endpoint = args[1];
    Value payload = args[2];
    Value headers = args[3];

    if (!url.is_string()) {
        error("Function 'put' expects argument 'url' to be a string");
    }

    if (!endpoint.is_string()) {
        error("Function 'put' expects argument 'endpoint' to be a string");
    }

    if (!payload.is_object() && !payload.is_string()) {
        error("Function 'put' expects argument 'payload' to be an object or string");
    }

    if (!headers.is_object()) {
        error("Function 'put' expects argument 'headers' to be an object");
    }

    httplib::Client cli(url.get_string());
    cli.enable_server_certificate_verification(false);

    httplib::Headers _headers;
    for (auto& prop : headers.get_object()->values) {
        _headers.insert({prop.first, toString(prop.second)});
    }

    httplib::Result res = httplib::Result(nullptr, httplib::Error());

    if (payload.is_object()) {
        httplib::MultipartFormDataItems items;
        for (auto& prop : payload.get_object()->values) {
            items.push_back({prop.first, toString(prop.second), "", ""});
        }
        res = cli.Put(endpoint.get_string(), _headers, items);
    } else {
        res = cli.Put(endpoint.get_string(), _headers, payload.get_string(), "");
    }

    Value response = object_val();

    if (!res) {
        response.get_object()->values["version"] = string_val("");
        response.get_object()->values["status"] = number_val(-1);
        response.get_object()->values["body"] = string_val("");
        response.get_object()->values["location"] = string_val("");
        response.get_object()->values["headers"] = list_val();
        return response;
    }

    response.get_object()->values["version"] = string_val(res->version);
    response.get_object()->values["status"] = number_val(res->status);
    response.get_object()->values["body"] = string_val(res->body);
    response.get_object()->values["location"] = string_val(res->location);
    response.get_object()->values["headers"] = list_val();
    for (auto& elem : res->headers) {
        response.get_object()->values["headers"].get_list()->push_back(string_val(elem.first));
    }

    return response;
}

extern "C" Value _patch(std::vector<Value>& args) {
    int num_required_args = 4;

    if (args.size() != num_required_args) {
        error("Function 'patch' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value url = args[0];
    Value endpoint = args[1];
    Value payload = args[2];
    Value headers = args[3];

    if (!url.is_string()) {
        error("Function 'patch' expects argument 'url' to be a string");
    }

    if (!endpoint.is_string()) {
        error("Function 'patch' expects argument 'endpoint' to be a string");
    }

    if (!payload.is_object() && !payload.is_string()) {
        error("Function 'patch' expects argument 'payload' to be an object or string");
    }

    if (!headers.is_object()) {
        error("Function 'patch' expects argument 'headers' to be an object");
    }

    httplib::Client cli(url.get_string());
    cli.enable_server_certificate_verification(false);

    httplib::Headers _headers;
    for (auto& prop : headers.get_object()->values) {
        _headers.insert({prop.first, toString(prop.second)});
    }

    std::string _payload;

    if (payload.is_string()) {
        _payload = payload.get_string();
    } else {
        _payload = toString(payload, true);
    }

    httplib::Result res = cli.Patch(endpoint.get_string(), _headers, _payload, "");

    Value response = object_val();

    if (!res) {
        response.get_object()->values["version"] = string_val("");
        response.get_object()->values["status"] = number_val(-1);
        response.get_object()->values["body"] = string_val("");
        response.get_object()->values["location"] = string_val("");
        response.get_object()->values["headers"] = list_val();
        return response;
    }

    response.get_object()->values["version"] = string_val(res->version);
    response.get_object()->values["status"] = number_val(res->status);
    response.get_object()->values["body"] = string_val(res->body);
    response.get_object()->values["location"] = string_val(res->location);
    response.get_object()->values["headers"] = list_val();
    for (auto& elem : res->headers) {
        response.get_object()->values["headers"].get_list()->push_back(string_val(elem.first));
    }

    return response;
}

extern "C" Value _delete(std::vector<Value>& args) {
    int num_required_args = 3;

    if (args.size() != num_required_args) {
        error("Function 'delete' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value url = args[0];
    Value endpoint = args[1];
    Value headers = args[2];

    if (!url.is_string()) {
        error("Function 'delete' expects argument 'url' to be a string");
    }

    if (!endpoint.is_string()) {
        error("Function 'delete' expects argument 'endpoint' to be a string");
    }

    if (!headers.is_object()) {
        error("Function 'delete' expects argument 'headers' to be a object");
    }

    httplib::Headers _headers;
    for (auto& prop : headers.get_object()->values) {
        _headers.insert({prop.first, toString(prop.second)});
    }

    httplib::Client cli(url.get_string());
    cli.enable_server_certificate_verification(false);

    auto res = cli.Delete(endpoint.get_string(), _headers);

    Value response = object_val();

    if (!res) {
        response.get_object()->values["version"] = string_val("");
        response.get_object()->values["status"] = number_val(-1);
        response.get_object()->values["body"] = string_val("");
        response.get_object()->values["location"] = string_val("");
        response.get_object()->values["headers"] = list_val();
        return response;
    }

    response.get_object()->values["version"] = string_val(res->version);
    response.get_object()->values["status"] = number_val(res->status);
    response.get_object()->values["body"] = string_val(res->body);
    response.get_object()->values["location"] = string_val(res->location);
    response.get_object()->values["headers"] = list_val();
    for (auto& elem : res->headers) {
        response.get_object()->values["headers"].get_list()->push_back(string_val(elem.first));
    }

    return response;
}

extern "C" Value _options(std::vector<Value>& args) {
    int num_required_args = 3;

    if (args.size() != num_required_args) {
        error("Function 'options' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value url = args[0];
    Value endpoint = args[1];
    Value headers = args[2];

    if (!url.is_string()) {
        error("Function 'options' expects argument 'url' to be a string");
    }

    if (!endpoint.is_string()) {
        error("Function 'options' expects argument 'endpoint' to be a string");
    }

    if (!headers.is_object()) {
        error("Function 'options' expects argument 'headers' to be a object");
    }

    httplib::Headers _headers;
    for (auto& prop : headers.get_object()->values) {
        _headers.insert({prop.first, toString(prop.second)});
    }

    httplib::Client cli(url.get_string());
    cli.enable_server_certificate_verification(false);

    auto res = cli.Options(endpoint.get_string(), _headers);

    Value response = object_val();

    if (!res) {
        response.get_object()->values["version"] = string_val("");
        response.get_object()->values["status"] = number_val(-1);
        response.get_object()->values["body"] = string_val("");
        response.get_object()->values["location"] = string_val("");
        response.get_object()->values["headers"] = list_val();
        return response;
    }

    response.get_object()->values["version"] = string_val(res->version);
    response.get_object()->values["status"] = number_val(res->status);
    response.get_object()->values["body"] = string_val(res->body);
    response.get_object()->values["location"] = string_val(res->location);
    response.get_object()->values["headers"] = list_val();
    for (auto& elem : res->headers) {
        response.get_object()->values["headers"].get_list()->push_back(string_val(elem.first));
    }

    return response;
}

extern "C" Value _head(std::vector<Value>& args) {
    int num_required_args = 3;

    if (args.size() != num_required_args) {
        error("Function 'head' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value url = args[0];
    Value endpoint = args[1];
    Value headers = args[2];

    if (!url.is_string()) {
        error("Function 'head' expects argument 'url' to be a string");
    }

    if (!endpoint.is_string()) {
        error("Function 'head' expects argument 'endpoint' to be a string");
    }

    if (!headers.is_object()) {
        error("Function 'head' expects argument 'headers' to be a object");
    }

    httplib::Headers _headers;
    for (auto& prop : headers.get_object()->values) {
        _headers.insert({prop.first, toString(prop.second)});
    }

    httplib::Client cli(url.get_string());
    cli.enable_server_certificate_verification(false);

    auto res = cli.Head(endpoint.get_string(), _headers);

    Value response = object_val();

    if (!res) {
        response.get_object()->values["version"] = string_val("");
        response.get_object()->values["status"] = number_val(-1);
        response.get_object()->values["body"] = string_val("");
        response.get_object()->values["location"] = string_val("");
        response.get_object()->values["headers"] = list_val();
        return response;
    }

    response.get_object()->values["version"] = string_val(res->version);
    response.get_object()->values["status"] = number_val(res->status);
    response.get_object()->values["body"] = string_val(res->body);
    response.get_object()->values["location"] = string_val(res->location);
    response.get_object()->values["headers"] = list_val();
    for (auto& elem : res->headers) {
        response.get_object()->values["headers"].get_list()->push_back(string_val(elem.first));
    }

    return response;
}

// VortexObj server(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 0;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     auto server = new httplib::Server();

//     server->set_exception_handler([](const auto& req, auto& res, std::exception_ptr ep) {
//         auto fmt = "<h1>Error 500</h1><p>%s</p>";
//         char buf[BUFSIZ];
//         try {
//             std::rethrow_exception(ep);
//         } catch (std::exception &e) {
//             snprintf(buf, sizeof(buf), fmt, e.what());
//         } catch (...) { // See the following NOTE
//             snprintf(buf, sizeof(buf), fmt, "Unknown Exception");
//         }
//         res.set_content(buf, "text/html");
//         res.status = 500;
//     });

//     VortexObj server_ptr = new_vortex_obj(NodeType::POINTER);
//     server_ptr->_Node.Pointer().value = server;
//     return server_ptr;
// }

// VortexObj server_ssl(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 2;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj cert_path = args[0];
//     VortexObj private_key_path = args[1];

//     if (cert_path->type != NodeType::STRING) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'cert_path' must be a string";
//         return error;
//     }

//     if (private_key_path->type != NodeType::STRING) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'private_key_path' must be a string";
//         return error;
//     }

//     auto server = new httplib::SSLServer(cert_path->_Node.String().value.c_str(), private_key_path->_Node.String().value.c_str());

//     server->set_exception_handler([](const auto& req, auto& res, std::exception_ptr ep) {
//         auto fmt = "<h1>Error 500</h1><p>%s</p>";
//         char buf[BUFSIZ];
//         try {
//             std::rethrow_exception(ep);
//         } catch (std::exception &e) {
//             snprintf(buf, sizeof(buf), fmt, e.what());
//         } catch (...) { // See the following NOTE
//             snprintf(buf, sizeof(buf), fmt, "Unknown Exception");
//         }
//         res.set_content(buf, "text/html");
//         res.status = 500;
//     });

//     VortexObj server_ptr = new_vortex_obj(NodeType::POINTER);
//     server_ptr->_Node.Pointer().value = server;
//     return server_ptr;
// }

// VortexObj start(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 4;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj v_server = args[0];
//     VortexObj v_host = args[1];
//     VortexObj v_port = args[2];
//     VortexObj v_ssl = args[3];

//     if (v_server->type != NodeType::POINTER) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'server' must be a pointer";
//         return error;
//     }

//     if (v_host->type != NodeType::STRING) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'host' must be a string";
//         return error;
//     }

//     if (v_port->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'port' must be a number";
//         return error;
//     }

//     if (v_ssl->type != NodeType::BOOLEAN) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'ssl' must be a boolean";
//         return error;
//     }

//     if (v_ssl->_Node.Boolean().value == true) {
//         httplib::SSLServer* svr = (httplib::SSLServer*)v_server->_Node.Pointer().value;
//         svr->listen(v_host->_Node.String().value, v_port->_Node.Number().value);
//         return new_vortex_obj(NodeType::NONE);
//     }

//     httplib::Server* svr = (httplib::Server*)v_server->_Node.Pointer().value;
//     svr->listen(v_host->_Node.String().value, v_port->_Node.Number().value);
//     return new_vortex_obj(NodeType::NONE);
// }

// std::string parse_route(std::string& route, std::vector<std::string>& params) {

//     // We want to parse the route in case we have parameters
//     // If we do, we store the param names and replace the param with
//     // the following regex: ([a-zA-Z0-9_-]+)
//     // If we have matches, we insert the matches into the callback req
//     // With the correct names we pulled out earlier

//     std::string parsed_route;
//     std::string regex = "([a-zA-Z0-9_-]+)";

//     for (int i = 0; i < route.size(); i++) {
//         if (route[i] == '{') {
//             std::string param;
//             i++;
//             parsed_route += regex;
//             while (route[i] != '}') {
//                 if (route[i] == '{' || !(isalpha(route[i]) || route[i] == '_')) {
//                     // No nesting allowed
//                     return "";
//                 }
//                 if (i == route.size()) {
//                     return "";
//                 }
//                 param += route[i];
//                 i++;
//             }
//             // Bypass '}'
//             i++;
//             params.push_back(param);
//         }

//         if (i < route.size()) {
//             parsed_route += route[i];
//         }
//     }

//     return parsed_route;
// }

// VortexObj set_get(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 5;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj v_server = args[0];
//     VortexObj v_route = args[1];
//     VortexObj v_callback = args[2];
//     VortexObj v_content_type = args[3];
//     VortexObj v_ssl = args[4];

//     if (v_server->type != NodeType::POINTER) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'server' must be a pointer";
//         return error;
//     }

//     if (v_route->type != NodeType::STRING) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'route' must be a string";
//         return error;
//     }

//     if (v_route->_Node.String().value.size() == 0 || v_route->_Node.String().value[0] != '/') {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'route' must begin with '/'";
//         return error;
//     }

//     if (v_callback->type != NodeType::FUNC) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'callback' must be a function";
//         return error;
//     }

//     if (v_content_type->type != NodeType::STRING) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'content_type' must be a string";
//         return error;
//     }

//     if (v_ssl->type != NodeType::BOOLEAN) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'ssl' must be a boolean";
//         return error;
//     }

//     std::vector<std::string> params;
//     auto route = v_route->_Node.String().value;
//     auto parsed_route = parse_route(route, params);

//     if (v_ssl->_Node.Boolean().value == true) {
//         httplib::SSLServer* svr = (httplib::SSLServer*)v_server->_Node.Pointer().value;

//         svr->Get(parsed_route, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type), params = std::move(params)](const httplib::Request &req, httplib::Response &res) {

//             VortexObj req_object = new_vortex_obj(NodeType::OBJECT);

//             if (v_callback->_Node.Function().params.size() == 1) {
//                 req_object->_Node.Object().properties["url_params"] = new_vortex_obj(NodeType::OBJECT);
//                 req_object->_Node.Object().properties["route_params"] = new_vortex_obj(NodeType::OBJECT);

//                 for (auto& url_param : req.params) {
//                     req_object->_Node.Object().properties["url_params"]->_Node.Object().properties[url_param.first] = new_string_node(url_param.second);
//                 }

//                 for (int i = 0; i < params.size(); i++) {
//                     auto param = req.matches[i+1];
//                     req_object->_Node.Object().properties["route_params"]->_Node.Object().properties[params[i]] = new_string_node(param);
//                 }
//             }

//             VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
//             if (v_callback->_Node.Function().params.size() == 1) {
//                 func_call->_Node.FunctionCall().args.push_back(req_object);
//             }
//             func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;
//             VortexObj result = interp.eval_func_call(func_call, v_callback);
            
//             if (result->type == NodeType::ERROR) {
//                 throw std::runtime_error(result->_Node.Error().message);
//             }

//             if (result->type == NodeType::OBJECT && result->TypeInfo.type_name == "Redirect") {
//                 std::vector<std::string> params;
//                 if (!result->_Node.Object().properties.count("route")) {
//                     throw std::runtime_error("Redirect object must contain 'route'");
//                 }

//                 VortexObj redirect_route_node = result->_Node.Object().properties["route"];
//                 if (redirect_route_node->type != NodeType::STRING) {
//                     throw std::runtime_error("Redirect object property 'route' must be a string");
//                 }

//                 std::string parsed_route = parse_route(redirect_route_node->_Node.String().value, params);

//                 res.set_redirect(parsed_route);
//                 return;
//             }

//             std::string content_type = v_content_type->_Node.String().value;
//             std::string result_string;
//             if (content_type == "text/html") {
//                 result_string = to_string(result, true);
//             } else {
//                 result_string = to_string(result);
//             }
//             res.set_content(result_string, content_type);
//         });

//         return new_vortex_obj(NodeType::NONE);
//     }

//     httplib::Server* svr = (httplib::Server*)v_server->_Node.Pointer().value;

//     svr->Get(parsed_route, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type), params = std::move(params)](const httplib::Request &req, httplib::Response &res) {
//         VortexObj req_object = new_vortex_obj(NodeType::OBJECT);

//         if (v_callback->_Node.Function().params.size() == 1) {
//             req_object->_Node.Object().properties["url_params"] = new_vortex_obj(NodeType::OBJECT);
//             req_object->_Node.Object().properties["route_params"] = new_vortex_obj(NodeType::OBJECT);

//             for (auto& url_param : req.params) {
//                 req_object->_Node.Object().properties["url_params"]->_Node.Object().properties[url_param.first] = new_string_node(url_param.second);
//             }

//             for (int i = 0; i < params.size(); i++) {
//                 auto param = req.matches[i+1];
//                 req_object->_Node.Object().properties["route_params"]->_Node.Object().properties[params[i]] = new_string_node(param);
//             }
//         }

//         VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
//         if (v_callback->_Node.Function().params.size() == 1) {
//             func_call->_Node.FunctionCall().args.push_back(req_object);
//         }

//         func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;
//         VortexObj result = interp.eval_func_call(func_call, v_callback);
        
//         if (result->type == NodeType::ERROR) {
//             throw std::runtime_error(result->_Node.Error().message);
//         }

//         if (result->type == NodeType::OBJECT && result->TypeInfo.type_name == "Redirect") {
//             std::vector<std::string> params;
//             if (!result->_Node.Object().properties.count("route")) {
//                 throw std::runtime_error("Redirect object must contain 'route'");
//             }

//             VortexObj redirect_route_node = result->_Node.Object().properties["route"];
//             if (redirect_route_node->type != NodeType::STRING) {
//                 throw std::runtime_error("Redirect object property 'route' must be a string");
//             }

//             std::string parsed_route = parse_route(redirect_route_node->_Node.String().value, params);

//             res.set_redirect(parsed_route);
//             return;
//         }

//         std::string content_type = v_content_type->_Node.String().value;
//         std::string result_string;
//         if (content_type == "text/html") {
//             result_string = to_string(result, true);
//         } else {
//             result_string = to_string(result);
//         }
//         res.set_content(result_string, content_type);
//     });

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj set_post(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 5;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj v_server = args[0];
//     VortexObj v_route = args[1];
//     VortexObj v_callback = args[2];
//     VortexObj v_content_type = args[3];
//     VortexObj v_ssl = args[4];

//     if (v_server->type != NodeType::POINTER) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'server' must be a pointer";
//         return error;
//     }

//     if (v_route->type != NodeType::STRING) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'route' must be a string";
//         return error;
//     }

//     if (v_route->_Node.String().value.size() == 0 || v_route->_Node.String().value[0] != '/') {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'route' must begin with '/'";
//         return error;
//     }

//     if (v_callback->type != NodeType::FUNC) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'callback' must be a function";
//         return error;
//     }

//     if (v_callback->_Node.Function().params.size() != 1) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'callback' must have one parameter";
//         return error;
//     }

//     if (v_content_type->type != NodeType::STRING) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'content_type' must be a string";
//         return error;
//     }

//     if (v_ssl->type != NodeType::BOOLEAN) {
//         VortexObj error = new_vortex_obj(NodeType::ERROR);
//         error->_Node.Error().message = "Parameter 'ssl' must be a boolean";
//         return error;
//     }

//     std::vector<std::string> params;
//     auto route = v_route->_Node.String().value;
//     auto parsed_route = parse_route(route, params);

//     if (v_ssl->_Node.Boolean().value == true) {
//         httplib::SSLServer* svr = (httplib::SSLServer*)v_server->_Node.Pointer().value;

//         svr->Post(parsed_route, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type), params = std::move(params)](const httplib::Request &req, httplib::Response &res) {
//             VortexObj req_object = new_vortex_obj(NodeType::OBJECT);
//             req_object->_Node.Object().properties["route_params"] = new_vortex_obj(NodeType::OBJECT);
//             req_object->_Node.Object().properties["data"] = new_vortex_obj(NodeType::OBJECT);

//             if (req.params.size() > 0) {   
//                 for (auto& param : req.params) {
//                     req_object->_Node.Object().properties["data"]->_Node.Object().properties[param.first] = new_string_node(param.second);
//                 }
//             } else if (req.files.size() > 0) {
//                 req_object->_Node.Object().properties["data"] = new_string_node(req.get_file_value("data").content);
//             }

//             for (int i = 0; i < params.size(); i++) {
//                 auto param = req.matches[i+1];
//                 req_object->_Node.Object().properties["route_params"]->_Node.Object().properties[params[i]] = new_string_node(param);
//             }

//             VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
//             func_call->_Node.FunctionCall().args.push_back(req_object);
//             func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;

//             VortexObj result = interp.eval_func_call(func_call, v_callback);

//             if (result->type == NodeType::ERROR) {
//                 throw std::runtime_error(result->_Node.Error().message);
//             }

//             if (result->type == NodeType::OBJECT && result->TypeInfo.type_name == "Redirect") {
//                 std::vector<std::string> params;
//                 if (!result->_Node.Object().properties.count("route")) {
//                     throw std::runtime_error("Redirect object must contain 'route'");
//                 }

//                 VortexObj redirect_route_node = result->_Node.Object().properties["route"];
//                 if (redirect_route_node->type != NodeType::STRING) {
//                     throw std::runtime_error("Redirect object property 'route' must be a string");
//                 }

//                 std::string parsed_route = parse_route(redirect_route_node->_Node.String().value, params);

//                 res.set_redirect(parsed_route);
//                 return;
//             }

//             std::string content_type = v_content_type->_Node.String().value;
//             std::string result_string;

//             if (content_type == "text/html") {
//                 result_string = to_string(result, true);
//             } else {
//                 result_string = to_string(result);
//             }

//             res.set_content(result_string, content_type);
//         });

//         return new_vortex_obj(NodeType::NONE);
//     }

//     httplib::Server* svr = (httplib::Server*)v_server->_Node.Pointer().value;

//     svr->Post(parsed_route, [v_callback = std::move(v_callback), v_content_type = std::move(v_content_type), params = std::move(params)](const httplib::Request &req, httplib::Response &res) {
//         VortexObj req_object = new_vortex_obj(NodeType::OBJECT);
//         req_object->_Node.Object().properties["route_params"] = new_vortex_obj(NodeType::OBJECT);
//         req_object->_Node.Object().properties["data"] = new_vortex_obj(NodeType::OBJECT);

//         if (req.params.size() > 0) {
//             for (auto& param : req.params) {
//                 req_object->_Node.Object().properties["data"]->_Node.Object().properties[param.first] = new_string_node(param.second);
//             }
//         } else if (req.files.size() > 0) {
//             req_object->_Node.Object().properties["data"] = new_string_node(req.get_file_value("data").content);
//         }

//         for (int i = 0; i < params.size(); i++) {
//             auto param = req.matches[i+1];
//             req_object->_Node.Object().properties["route_params"]->_Node.Object().properties[params[i]] = new_string_node(param);
//         }

//         VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
//         func_call->_Node.FunctionCall().args.push_back(req_object);
//         func_call->_Node.FunctionCall().name = v_callback->_Node.Function().name;

//         VortexObj result = interp.eval_func_call(func_call, v_callback);

//         if (result->type == NodeType::OBJECT && result->TypeInfo.type_name == "Redirect") {
//             std::vector<std::string> params;
//             if (!result->_Node.Object().properties.count("route")) {
//                 throw std::runtime_error("Redirect object must contain 'route'");
//             }

//             VortexObj redirect_route_node = result->_Node.Object().properties["route"];
//             if (redirect_route_node->type != NodeType::STRING) {
//                 throw std::runtime_error("Redirect object property 'route' must be a string");
//             }

//             std::string parsed_route = parse_route(redirect_route_node->_Node.String().value, params);

//             res.set_redirect(parsed_route);
//             return;
//         }

//         if (result->type == NodeType::ERROR) {
//             throw std::runtime_error(result->_Node.Error().message);
//         }

//         std::string content_type = v_content_type->_Node.String().value;
//         std::string result_string;

//         if (content_type == "text/html") {
//             result_string = to_string(result, true);
//         } else {
//             result_string = to_string(result);
//         }

//         res.set_content(result_string, content_type);
//     });

//     return new_vortex_obj(NodeType::NONE);
// }

// /* Implement call_function */

// extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
//     if (name == "get") {
//         return get(name, args);
//     }
//     if (name == "post") {
//         return post(name, args);
//     }
//     if (name == "server") {
//         return server(name, args);
//     }
//     if (name == "server_ssl") {
//         return server_ssl(name, args);
//     }
//     if (name == "start") {
//         return start(name, args);
//     }
//     if (name == "set_get") {
//         return set_get(name, args);
//     }
//     if (name == "set_post") {
//         return set_post(name, args);
//     }

//     error_and_exit("Function '" + name + "' is undefined");

//     return new_vortex_obj(NodeType::NONE);
// }