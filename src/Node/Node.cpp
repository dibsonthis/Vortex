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
    }
}