#include "Node.hpp"

node_ptr new_number_node(double value) {
    auto node = std::make_shared<Node>(NodeType::NUMBER);
    node->Number.value = value;
    return node;
}

node_ptr new_string_node(std::string value) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->String.value = value;
    return node;
}

node_ptr new_boolean_node(bool value) {
    auto node = std::make_shared<Node>(NodeType::BOOLEAN);
    node->Boolean.value = value;
    return node;
}