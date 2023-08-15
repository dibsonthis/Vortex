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
        add_opcode(chunk, OP_SET, index, node->line);
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
    add_opcode(chunk, OP_LOAD, index, node->line);
}

void gen_if(Chunk& chunk, node_ptr node) {
    generate(node->_Node.IfStatement().condition, chunk);
    begin_scope();
    int jump_instruction = chunk.code.size() + 1;
    add_opcode(chunk, OP_POP_JUMP_IF_FALSE, 0, node->line);
    generate_bytecode(node->_Node.IfStatement().body->_Node.Object().elements, chunk);
    int offset = chunk.code.size() - jump_instruction - 4;
    uint8_t* bytes = int_to_bytes(offset);
    patch_bytes(chunk, jump_instruction, bytes);
    end_scope(chunk);
}

void gen_if_block(Chunk& chunk, node_ptr node) {
    gen_if(chunk, node->_Node.IfBlock().statements[0]);
    for (int i = 1; i < node->_Node.IfBlock().statements.size(); i++) {
        node_ptr& statement = node->_Node.IfBlock().statements[i];
        if (statement->type == NodeType::IF_STATEMENT) {
            gen_if(chunk, statement);
        } else if (statement->type == NodeType::OBJECT) {
            generate_bytecode(statement->_Node.Object().elements, chunk);
        }
    }
}

void gen_while_loop(Chunk& chunk, node_ptr node) {
    int start_index = chunk.code.size() - 1;
    generate(node->_Node.WhileLoop().condition, chunk);
    int jump_instruction = chunk.code.size() + 1;
    add_opcode(chunk, OP_POP_JUMP_IF_FALSE, 0, node->line);
    begin_scope();
    current.in_loop = true;
    generate_bytecode(node->_Node.WhileLoop().body->_Node.Object().elements, chunk);
    end_scope(chunk);
    current.in_loop = false;
    add_opcode(chunk, OP_JUMP_BACK, chunk.code.size() - start_index + 4, node->_Node.WhileLoop().body->_Node.Object().elements.back()->line);
    int offset = chunk.code.size() - jump_instruction - 4;
    uint8_t* bytes = int_to_bytes(offset);
    patch_bytes(chunk, jump_instruction, bytes);
}

void gen_break(Chunk& chunk, node_ptr node) {
    if (!current.in_loop) {
        error("Cannot use 'break' outside of a loop");
    }

    add_code(chunk, OP_BREAK, node->line);
}

void gen_continue(Chunk& chunk, node_ptr node) {
    if (!current.in_loop) {
        error("Cannot use 'continue' outside of a loop");
    }

    add_code(chunk, OP_CONTINUE, node->line);
}

void gen_and(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    int jump_instruction = chunk.code.size() + 1;
    add_opcode(chunk, OP_JUMP_IF_FALSE, 0, node->line);
    add_code(chunk, OP_POP, node->line);
    generate(node->_Node.Op().right, chunk);
    int offset = chunk.code.size() - jump_instruction - 4;
    uint8_t* bytes = int_to_bytes(offset);
    patch_bytes(chunk, jump_instruction, bytes);
}

void gen_or(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    int jump_instruction = chunk.code.size() + 1;
    add_opcode(chunk, OP_JUMP_IF_TRUE, 0, node->line);
    add_code(chunk, OP_POP, node->line);
    generate(node->_Node.Op().right, chunk);
    int offset = chunk.code.size() - jump_instruction - 4;
    uint8_t* bytes = int_to_bytes(offset);
    patch_bytes(chunk, jump_instruction, bytes);
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
        case NodeType::IF_STATEMENT: {
            gen_if(chunk, node);
            break;
        }
        case NodeType::IF_BLOCK: {
            gen_if_block(chunk, node);
            break;
        }
        case NodeType::WHILE_LOOP: {
            gen_while_loop(chunk, node);
            break;
        }
        case NodeType::BREAK: {
            gen_break(chunk, node);
            break;
        }
        case NodeType::CONTINUE: {
            gen_continue(chunk, node);
            break;
        }
    }

    if (node->type == NodeType::OP) {
        if (node->_Node.Op().value == "-" && !node->_Node.Op().left) {
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
        if (node->_Node.Op().value == "-=") {
            node_ptr sub_node = std::make_shared<Node>(NodeType::OP);
            sub_node->_Node.Op().value = "-";
            sub_node->_Node.Op().left = node->_Node.Op().left;
            sub_node->_Node.Op().right = node->_Node.Op().right;

            node_ptr eq_node = std::make_shared<Node>(NodeType::OP);
            eq_node->_Node.Op().value = "=";
            eq_node->_Node.Op().left = node->_Node.Op().left;
            eq_node->_Node.Op().right = sub_node;

            gen_eq(chunk, eq_node);
            return;
        }
        if (node->_Node.Op().value == "+=") {
            node_ptr add_node = std::make_shared<Node>(NodeType::OP);
            add_node->_Node.Op().value = "+";
            add_node->_Node.Op().left = node->_Node.Op().left;
            add_node->_Node.Op().right = node->_Node.Op().right;

            node_ptr eq_node = std::make_shared<Node>(NodeType::OP);
            eq_node->_Node.Op().value = "=";
            eq_node->_Node.Op().left = node->_Node.Op().left;
            eq_node->_Node.Op().right = add_node;

            gen_eq(chunk, eq_node);
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
        if (node->_Node.Op().value == "and") {
            gen_and(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "or") {
            gen_or(chunk, node);
            return;
        }
    }
}

void generate_bytecode(std::vector<node_ptr>& nodes, Chunk& chunk) {
    for (node_ptr& node : nodes) {
        if ((node->type == NodeType::OP && node->_Node.Op().value != "=" && node->_Node.Op().value != "+=" && node->_Node.Op().value != "-=") ||
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