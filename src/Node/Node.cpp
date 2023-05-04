#pragma once
#include "Node.hpp"

std::string node_repr(node_ptr node) {
    switch (node->type) {
        case NodeType::ID: {
            return node->ID.value;
        }
        case NodeType::NUMBER: {
            return "Number";
        }
        case NodeType::STRING: {
            return "String";
        }
        case NodeType::BOOLEAN: {
            return "Boolean";
        }
        case NodeType::LIST: {
            if (node->List.elements.size() == 0) {
                return "List";
            } else if (node->List.elements.size() == 1) {
                return "[" + node_repr(node->List.elements[0]) + "]";
            } else if (node->List.elements.size() > 10) {
                std::string res = "[";
                for (int i = 0; i < 10; i++) {
                    res += node_repr(node->List.elements[i]);
                    if (i != 9) {
                        res += ", ";
                    }
                }
                res += " ... ]";
                return res;
            }
            std::string res = "[";
            for (int i = 0; i < node->List.elements.size(); i++) {
                res += node_repr(node->List.elements[i]);
                if (i < node->List.elements.size()-1) {
                    res += ", ";
                }
            }
            res += "]";
            return res;
        }
        case NodeType::OBJECT: {
            if (node->TypeInfo.type) {
                return node->TypeInfo.type_name;
            }
            if (node->TypeInfo.is_type && node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            if (node->Object.is_enum && node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            return "Object";
        }
        case NodeType::LIB: {
            return "Library";
        }
        case NodeType::START_OF_FILE: {
            return "SOF";
        }
        case NodeType::END_OF_FILE: {
            return "EOF";
        }
        case NodeType::CONSTANT_DECLARATION: {
            return "Const declaration";
        }
        case NodeType::CONSTANT_DECLARATION_MULTIPLE: {
            return "Const declaration";
        }
        case NodeType::VARIABLE_DECLARATION: {
            return "Variable declaration";
        }
        case NodeType::VARIABLE_DECLARATION_MULTIPLE: {
            return "Variable declaration";
        }
        case NodeType::FUNC_CALL: {
            return "Function call";
        }
        case NodeType::FUNC: {
            if (node->Function.name != "") {
                return node->Function.name;
            }
            return "Function";
        }
        case NodeType::IMPORT: {
            return "Import statement";
        }
        case NodeType::POINTER: {
            return "Pointer";
        }
        case NodeType::NONE: {
            return "None";
        }
        default: {
            return "<not_implemented>";
        }
    }
}