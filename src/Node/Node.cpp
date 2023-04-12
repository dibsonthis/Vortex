#include "Node.hpp"

std::string node_repr(node_ptr node) {
    switch (node->type) {
        case NodeType::ID: {
            return node->ID.value;
        }
        case NodeType::NUMBER: {
            return std::to_string(node->Number.value);
        }
        case NodeType::STRING: {
            return node->String.value;
        }
        case NodeType::OP: {
            return node->Operator.value;
        }
        case NodeType::BOOLEAN: {
            return node->Boolean.value ? "true" : "false";
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
        default: {
            return "<not_implemented>";
        }
    }
}