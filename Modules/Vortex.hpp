#include "../src/Node/Node.hpp"

#define VortexObj std::shared_ptr<Node>

void error_and_exit(std::string message) {
    std::cout << "Error: " << message;
    exit(1);
}

VortexObj new_vortex_obj(NodeType type) {
    auto node = std::make_shared<Node>(type);
    return node;
}

VortexObj new_number_node(double number) {
    auto node = std::make_shared<Node>(NodeType::NUMBER);
    node->Number.value = number;
    return node;
}

VortexObj new_string_node(std::string text) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->String.value = text;
    return node;
}