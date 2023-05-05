#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN
#include "../include/httplib.h"
#include "../include/json.hpp"
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

    httplib::Client cli(args[0]->String.value);

    auto res = cli.Get(args[1]->String.value);

    if (!res) {
        error_and_exit("Failed request: " + httplib::to_string(res.error()));
    }

    VortexObj response = new_vortex_obj(NodeType::OBJECT);
    response->Object.properties["version"] = new_string_node(res->version);
    response->Object.properties["status"] = new_number_node(res->status);
    response->Object.properties["body"] = new_string_node(res->body);
    response->Object.properties["location"] = new_string_node(res->location);
    response->Object.properties["headers"] = new_vortex_obj(NodeType::LIST);
    for (auto& elem : res->headers) {
        response->Object.properties["headers"]->List.elements.push_back(new_string_node(elem.first));
    }

    return response;
}

VortexObj json_type_to_vtx_type(nlohmann::json_abi_v3_11_2::json json_obj) {

    auto type = json_obj.type();

    if (type == nlohmann::json::value_t::string) {
        return new_string_node(json_obj);
    }
    if (type == nlohmann::json::value_t::boolean) {
        VortexObj boolean = new_vortex_obj(NodeType::BOOLEAN);
        boolean->Boolean.value = json_obj;
        return boolean;
    }
    if (type == nlohmann::json::value_t::number_integer || type == nlohmann::json::value_t::number_unsigned || type == nlohmann::json::value_t::number_float) {
        return new_number_node(json_obj);
    }
    if (type == nlohmann::json::value_t::null) {
        return new_vortex_obj(NodeType::NONE);
    }
    if (type == nlohmann::json::value_t::array) {
        VortexObj list = new_vortex_obj(NodeType::LIST);
        for (auto val : json_obj) {
            list->List.elements.push_back(json_type_to_vtx_type(val));
        }
        return list;
    }
    if (type == nlohmann::json::value_t::object) {
        VortexObj object = new_vortex_obj(NodeType::OBJECT);
        for (auto val : json_obj.items()) {
            object->Object.properties[val.key()] = json_type_to_vtx_type(val.value());
        }
        return object;
    }

    return new_vortex_obj(NodeType::NONE);
}

VortexObj to_json(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    if (args[0]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }
    auto json = nlohmann::json::parse(args[0]->String.value);
    VortexObj json_obj = new_vortex_obj(NodeType::OBJECT);
    for (auto& prop : json.items()) {
        auto value = prop.value();
        auto key = prop.key();
        json_obj->Object.properties[key] = json_type_to_vtx_type(value);
    }
    return json_obj;
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "get") {
        return get(name, args);
    }
    if (name == "to_json") {
        return to_json(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}