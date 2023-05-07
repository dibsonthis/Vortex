#include "../include/json.hpp"
#include "../../Vortex.hpp"

int indent_level = 0;
std::string indent = "    ";

std::string to_string(VortexObj node) {
    switch (node->type) {
        case NodeType::NUMBER: {
            std::string num_str = std::to_string(node->Number.value);
            num_str.erase(num_str.find_last_not_of('0') + 1, std::string::npos);
            num_str.erase(num_str.find_last_not_of('.') + 1, std::string::npos);
            return num_str;
        }
        case NodeType::BOOLEAN: {
            return node->Boolean.value ? "true" : "false";
        }
        case NodeType::STRING: {
            return '"' + node->String.value + '"';
        }
        case NodeType::LIST: {
            if (node->List.elements.size() == 0) {
                return "[]";
            }
            std::string res = "";
            res += "[\n";
            indent_level++;
            for (int i = 0; i < node->List.elements.size(); i++) {
                for (int ind = 0; ind < indent_level; ind++) {
                    res += indent;
                }
                res += to_string(node->List.elements[i]);
                if (i < node->List.elements.size()-1) {
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
            if (node->Object.properties.size() == 0) {
                return "{}";
            }
            std::string res = "";
            res += "{\n";
            indent_level++;
            int elem_i = 0;
            for (auto const& elem : node->Object.properties) {
                for (int i = 0; i < indent_level; i++) {
                    res += indent;
                }
                res +=  '"' + elem.first + '"' + ": " + to_string(elem.second);
                if (elem_i != node->Object.properties.size()-1) {
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
            return '"' + node->ID.value + '"';
        }
        case NodeType::OP: {
            if (node->Operator.value == ".") {
                return to_string(node->Operator.left) + "." + to_string(node->Operator.right);
            }
            return node->Operator.value;
        }
        default: {
            return "";
        }
    }
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

/* Implement Lib Functions */

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

VortexObj serialize(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    if (args[0]->type != NodeType::OBJECT && args[0]->type != NodeType::LIST) {
        error_and_exit("Function '" + name + "' expects 1 object or list argument");
    }

    VortexObj serialized_object = new_string_node(to_string(args[0]));
    return serialized_object;
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "parse") {
        return parse(name, args);
    }
    if (name == "serialize") {
        return serialize(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}