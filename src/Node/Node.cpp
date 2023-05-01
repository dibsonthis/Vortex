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
            return "List";
        }
        case NodeType::OBJECT: {
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
        case NodeType::IMPORT: {
            return "Import statement";
        }
        case NodeType::NONE: {
            return "None";
        }
        default: {
            return "<not_implemented>";
        }
    }
}