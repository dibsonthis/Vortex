#include "../include/json.hpp"
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

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

VortexObj parse(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    if (args[0]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    std::string json_body = args[0]->String.value;

    try {
        auto json = nlohmann::json::parse(json_body);
        VortexObj json_obj = new_vortex_obj(NodeType::OBJECT);
        for (auto& prop : json.items()) {
            auto value = prop.value();
            auto key = prop.key();
            json_obj->Object.properties[key] = json_type_to_vtx_type(value);
        }
        return json_obj;
    } catch (const std::exception &exc) {
        return new_vortex_obj(NodeType::OBJECT);
    }
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "parse") {
        return parse(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}