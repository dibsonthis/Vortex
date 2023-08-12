#include "Generator.hpp"
#include "Bytecode.hpp"
#include "../Node/Node.hpp"

Compiler current;

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
        case NodeType::NONE: {
            add_constant_code(chunk, none_val(), node->line);
            break;
        }
    }
}

void gen_paren(Chunk& chunk, node_ptr node) {
    if (node->_Node.Paren().elements.size() != 0) {
        generate(node->_Node.Paren().elements[0], chunk);
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

void gen_eq_eq(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_EQ_EQ, node->line);
}

void gen_not_eq(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_NOT_EQ, node->line);
}

void gen_lt_eq(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_LT_EQ, node->line);
}

void gen_gt_eq(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_GT_EQ, node->line);
}

void gen_lt(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_LT, node->line);
}

void gen_gt(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_GT, node->line);
}

void gen_eq(Chunk& chunk, node_ptr node) {
    node_ptr left = node->_Node.Op().left;
    if (left->type == NodeType::ID) {
        int index = resolve_variable(left->_Node.ID().value);
        Variable variable = current.variables[index];
        if (variable.is_const) {
            error("Cannot modify constant '" + left->_Node.ID().value + "'");
        }
        generate(node->_Node.Op().right, chunk);
        add_bytes(chunk, number_val(index), OP_SET, node->line);
    }
}

void gen_var(Chunk& chunk, node_ptr node) {
    generate(node->_Node.VariableDeclaration().value, chunk);
    declareVariable(node->_Node.VariableDeclaration().name);
}

void gen_const(Chunk& chunk, node_ptr node) {
    generate(node->_Node.ConstantDeclatation().value, chunk);
    declareVariable(node->_Node.ConstantDeclatation().name, true);
}

void gen_id(Chunk& chunk, node_ptr node) {
    int index = resolve_variable(node->_Node.ID().value);
    if (index == -1) {
        error("Variable '" + node->_Node.ID().value + "' is undefined");
    }
    add_bytes(chunk, number_val(index), OP_LOAD, node->line);
}

void generate(node_ptr node, Chunk& chunk) {
    switch (node->type) {
        case NodeType::NUMBER:
        case NodeType::STRING:
        case NodeType::BOOLEAN:
        case NodeType::NONE: {
            gen_literal(chunk, node);
            break;
        }
        case NodeType::ID: {
            gen_id(chunk, node);
            break;
        }
        case NodeType::PAREN: {
            gen_paren(chunk, node);
            break;
        }
        case NodeType::VARIABLE_DECLARATION: {
            gen_var(chunk, node);
            break;
        }
        case NodeType::CONSTANT_DECLARATION: {
            gen_const(chunk, node);
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
        if (node->_Node.Op().value == "==") {
            gen_eq_eq(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "!=") {
            gen_not_eq(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "<=") {
            gen_lt_eq(chunk, node);
            return;
        }
        if (node->_Node.Op().value == ">=") {
            gen_gt_eq(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "<") {
            gen_lt(chunk, node);
            return;
        }
        if (node->_Node.Op().value == ">") {
            gen_gt(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "=") {
            gen_eq(chunk, node);
            return;
        }
    }
}

void generate_bytecode(std::vector<node_ptr>& nodes, Chunk& chunk) {
    for (node_ptr& node : nodes) {
        if ((node->type == NodeType::OP && node->_Node.Op().value != "=") ||
            node->type == NodeType::STRING ||
            node->type == NodeType::NUMBER ||
            node->type == NodeType::BOOLEAN ||
            node->type == NodeType::LIST ||
            node->type == NodeType::OBJECT) {
            continue;
        }
        generate(node, chunk);
    }
}

static void begin_scope() {
    current.scopeDepth++;
}

static void end_scope(Chunk& chunk) {
    current.scopeDepth--;

    while (current.variableCount > 0 
    && current.variables[current.variableCount - 1].depth > current.scopeDepth) {
        add_code(chunk, OP_POP);
        current.variableCount--;
  }
}

static void addVariable(std::string name, bool is_const) {
    current.variableCount++;
    Variable variable;
    variable.name = name;
    variable.depth = current.scopeDepth;
    variable.is_const = is_const;
    current.variables.push_back(variable);
}

static void declareVariable(std::string name, bool is_const) {

    for (int i = current.variableCount - 1; i >= 0; i--) {
        Variable variable = current.variables[i];
        if (variable.depth != -1 && variable.depth < current.scopeDepth) {
            break; 
        }

        if (name == variable.name) {
            error("Variable '" + name + "' already defined");
        }
    }

    addVariable(name, is_const);
}

static int resolve_variable(std::string name) {
    for (int i = current.variableCount - 1; i >= 0; i--) {
        Variable variable = current.variables[i];
        if (name == variable.name) {
            return i;
        }
    }

    return -1;
}

void error(std::string message) {
    std::cout << message << "\n";
    exit(1);
}