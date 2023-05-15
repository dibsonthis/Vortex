#pragma once
#include "Node.hpp"

std::string node_repr(node_ptr node) {
    switch (node->type) {
        case NodeType::ID: {
            return node->_Node.ID().value;
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
            if (node->_Node.List().is_union) {
                return node->TypeInfo.type_name;
            }
            if (node->_Node.List().elements.size() == 0) {
                return "List";
            } else if (node->_Node.List().elements.size() == 1) {
                return "[" + node_repr(node->_Node.List().elements[0]) + "]";
            } else if (node->_Node.List().elements.size() > 10) {
                std::string res = "[";
                for (int i = 0; i < 10; i++) {
                    res += node_repr(node->_Node.List().elements[i]);
                    if (i != 9) {
                        res += ", ";
                    }
                }
                res += " ... ]";
                return res;
            }
            std::string res = "[";
            for (int i = 0; i < node->_Node.List().elements.size(); i++) {
                res += node_repr(node->_Node.List().elements[i]);
                if (i < node->_Node.List().elements.size()-1) {
                    res += ", ";
                }
            }
            res += "]";
            return res;
        }
        case NodeType::OBJECT: {
            if (node->TypeInfo.type && node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            if (node->TypeInfo.is_type && node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            if (node->_Node.Object().is_enum && node->TypeInfo.type_name != "") {
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
        case NodeType::VARIABLE_DECLARATION: {
            return "Variable declaration";
        }
        case NodeType::FUNC_CALL: {
            return "Function call";
        }
        case NodeType::FUNC: {
            if (node->_Node.Function().name != "") {
                return node->_Node.Function().name;
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
        case NodeType::ERROR: {
            return "Error";
        }
        default: {
            return "<not_implemented>";
        }
    }
}