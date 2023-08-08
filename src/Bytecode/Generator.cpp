#include "Generator.hpp"
#include "Bytecode.hpp"
#include "../Node/Node.hpp"

void gen_literal(Chunk& chunk, node_ptr node) {
    switch (node->type) {
        case NodeType::NUMBER: {
            add_constant_code(chunk, number_val(node->_Node.Number().value), node->line);
            break;
        }
        case NodeType::STRING: {
            add_constant_code(chunk, string_val(node->_Node.String().value), node->line);
            break;
        }
        case NodeType::BOOLEAN: {
            add_constant_code(chunk, boolean_val(node->_Node.Boolean().value), node->line);
            break;
        }
    }
}

void gen_neg(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_NEGATE, node->line);
}

void gen_add(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_ADD, node->line);
}

void gen_multiply(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_MULTIPLY, node->line);
}

void gen_subtract(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_SUBTRACT, node->line);
}

void gen_divide(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_DIVIDE, node->line);
}

void gen_not(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_NOT, node->line);
}


void generate(node_ptr node, Chunk& chunk) {
    switch (node->type) {
        case NodeType::NUMBER:
        case NodeType::STRING:
        case NodeType::BOOLEAN: {
            gen_literal(chunk, node);
            break;
        }
    }

    if (node->type == NodeType::OP) {
        if (node->_Node.Op().value == "-") {
            gen_neg(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "+") {
            gen_add(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "*") {
            gen_multiply(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "-") {
            gen_subtract(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "/") {
            gen_divide(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "!") {
            gen_not(chunk, node);
            return;
        }
    }
}

void generate_bytecode(std::vector<node_ptr>& nodes, Chunk& chunk) {
    for (node_ptr& node : nodes) {
        generate(node, chunk);
    }
}