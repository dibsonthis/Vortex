#pragma once

#include "../src/Node/Node.cpp"

#define VortexObj std::shared_ptr<Node>

void error_and_exit(std::string message, VortexObj obj = nullptr) {
    if (obj) {
        std::string error_message = "\nError @ (" + std::to_string(obj->line) + ", " + std::to_string(obj->column) + "): " + message;
	    std::cout << error_message << "\n";
    } else {
        std::cout << "Error: " << message;
    }
    exit(1);
}

VortexObj new_vortex_obj(NodeType type) {
    auto node = std::make_shared<Node>(type);
    return node;
}

VortexObj new_number_node(double number) {
    auto node = std::make_shared<Node>(NodeType::NUMBER);
    node->_Node.Number().value = number;
    return node;
}

VortexObj new_boolean_node(bool value) {
    auto node = std::make_shared<Node>(NodeType::BOOLEAN);
    node->_Node.Boolean().value = value;
    return node;
}

VortexObj new_string_node(std::string text) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->_Node.String().value = text;
    return node;
}

VortexObj new_error_node(std::string text) {
    auto node = std::make_shared<Node>(NodeType::_ERROR);
    node->_Node.Error().message = text;
    return node;
}