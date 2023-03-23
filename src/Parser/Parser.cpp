#include "Parser.hpp"

void Parser::advance(int n) {
    index += n;
    if (index < nodes.size()) {
        current_node = nodes[index];
    }
}

void Parser::reset(int idx) {
    index = idx;
    current_node = nodes[index];
}

node_ptr Parser::get_left() {
    int idx = index;
    node_ptr node = nodes[idx - 1];
    while (node->__parsed) {
        idx--;
        node = nodes[idx];
    }
    return node;
}

node_ptr Parser::get_right() {
    int idx = index;
    node_ptr node = nodes[idx + 1];
    while (node->__parsed) {
        idx++;
        node = nodes[idx];
    }
    return node;
}

void Parser::parse_bin_op(std::vector<std::string> operators) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (vector_contains_string(operators, current_node->Operator.value)) {
            node_ptr left = get_left();
            node_ptr right = get_right();
            left->__parsed = true;
            right->__parsed = true;
            current_node->Operator.left = left;
            current_node->Operator.right = right;
        }
        advance();
    }

    reset();
}

std::vector<node_ptr> Parser::filter_tree() {
    std::vector<node_ptr> ast;
    for (node_ptr& node : nodes) {
        if (!node->__parsed) {
            ast.push_back(node);
        }
    }
    return ast;
}