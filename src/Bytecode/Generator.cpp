#include "Generator.hpp"
#include "Bytecode.hpp"
#include "../Node/Node.hpp"

std::shared_ptr<Compiler> current = std::make_shared<Compiler>();

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

void gen_list(Chunk& chunk, node_ptr node) {
    if (node->_Node.List().elements.size() == 1 && node->_Node.List().elements[0]->type == NodeType::COMMA_LIST) {
        node = node->_Node.List().elements[0];
    }

    for (node_ptr& elem : node->_Node.List().elements) {
        generate(elem, chunk);
    }
    add_opcode(chunk, OP_BUILD_LIST, node->_Node.List().elements.size(), node->line);
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
        if (index == -1) {
            error("Variable '" + left->_Node.ID().value + "' is undefined");
        }
        Variable variable = current->variables[index];
        if (variable.is_const) {
            error("Cannot modify constant '" + left->_Node.ID().value + "'");
        }
        generate(node->_Node.Op().right, chunk);
        add_opcode(chunk, OP_SET, index, node->line);
    }
}

void gen_range(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_RANGE, node->line);
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
        auto prev_compiler = current;
        if (!current->prev) {
            goto is_global;
        }
        current = current->prev;
        int index = resolve_variable(node->_Node.ID().value);
        if (index == -1) {
            goto is_global;
        }
        bool captured = false;
        for (int var : current->closed_vars) {
            if (index == var) {
                captured = true;
                break;
            }
        }
        if (!captured) {
            current->closed_vars.push_back(index);
            // closure_count++;
        }
        add_opcode(chunk, OP_LOAD_CLOSURE, index, node->line);
        current = prev_compiler;
        return;
    }
    add_opcode(chunk, OP_LOAD, index, node->line);
    return;

    is_global:
        add_constant_code(chunk, string_val(node->_Node.ID().value), node->line);
        add_code(chunk, OP_LOAD_GLOBAL, node->line);
        return;
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
    current->in_loop = true;
    generate_bytecode(node->_Node.WhileLoop().body->_Node.Object().elements, chunk);
    end_scope(chunk);
    current->in_loop = false;
    add_opcode(chunk, OP_JUMP_BACK, chunk.code.size() - start_index + 4, node->_Node.WhileLoop().body->_Node.Object().elements.back()->line);
    int offset = chunk.code.size() - jump_instruction - 4;
    uint8_t* bytes = int_to_bytes(offset);
    patch_bytes(chunk, jump_instruction, bytes);
}

void gen_for_loop(Chunk& chunk, node_ptr node) {
    if (!node->_Node.ForLoop().iterator) {
        if (!node->_Node.ForLoop().index_name) {
            node->_Node.ForLoop().index_name = std::make_shared<Node>(NodeType::ID);
            node->_Node.ForLoop().index_name->_Node.ID().value = "___index___";
        }

        begin_scope();

        add_constant_code(chunk, number_val(0));
        declareVariable(node->_Node.ForLoop().index_name->_Node.ID().value);

        if (node->_Node.ForLoop().value_name) {
            generate(node->_Node.ForLoop().start, chunk);
            declareVariable(node->_Node.ForLoop().value_name->_Node.ID().value);
        }

        generate(node->_Node.ForLoop().end, chunk);
        generate(node->_Node.ForLoop().start, chunk);
        add_code(chunk, OP_SUBTRACT, node->line);
        declareVariable("___size___");

        int loop_start = chunk.code.size() - 1;

        generate_bytecode(node->_Node.ForLoop().body->_Node.Object().elements, chunk);

        add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_constant_code(chunk, number_val(1), node->line);
        add_code(chunk, OP_ADD, node->line);
        add_opcode(chunk, OP_SET, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);

        if (node->_Node.ForLoop().value_name) {
            add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().value_name->_Node.ID().value), node->line);
            add_constant_code(chunk, number_val(1), node->line);
            add_code(chunk, OP_ADD, node->line);
            add_opcode(chunk, OP_SET, resolve_variable(node->_Node.ForLoop().value_name->_Node.ID().value), node->line);
        }

        add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_opcode(chunk, OP_LOAD, resolve_variable("___size___"));
        add_code(chunk, OP_GT_EQ, node->line);
        add_opcode(chunk, OP_POP_JUMP_IF_TRUE, 5, node->line);

        add_opcode(chunk, OP_JUMP_BACK, chunk.code.size() - loop_start + 4, node->line);

        end_scope(chunk);
    } else {
        if (!node->_Node.ForLoop().index_name) {
            node->_Node.ForLoop().index_name = std::make_shared<Node>(NodeType::ID);
            node->_Node.ForLoop().index_name->_Node.ID().value = "___index___";
        }

        begin_scope();

        add_constant_code(chunk, number_val(0));
        declareVariable(node->_Node.ForLoop().index_name->_Node.ID().value);

        generate(node->_Node.ForLoop().iterator, chunk);
        declareVariable("___iter___");

        if (node->_Node.ForLoop().value_name) {
            add_opcode(chunk, OP_LOAD, resolve_variable("___iter___"), node->line);
            add_constant_code(chunk, number_val(0));
            add_code(chunk, OP_ACCESSOR, node->line);
            declareVariable(node->_Node.ForLoop().value_name->_Node.ID().value);
        }

        add_opcode(chunk, OP_LOAD, resolve_variable("___iter___"), node->line);
        add_code(chunk, OP_LEN, node->line);
        declareVariable("___size___");

        int loop_start = chunk.code.size() - 1;

        generate_bytecode(node->_Node.ForLoop().body->_Node.Object().elements, chunk);

        add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_constant_code(chunk, number_val(1), node->line);
        add_code(chunk, OP_ADD, node->line);
        add_opcode(chunk, OP_SET, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);

        if (node->_Node.ForLoop().value_name) {
            add_opcode(chunk, OP_LOAD, resolve_variable("___iter___"), node->line);
            add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
            add_code(chunk, OP_ACCESSOR, node->line);
            add_opcode(chunk, OP_SET, resolve_variable(node->_Node.ForLoop().value_name->_Node.ID().value), node->line);
        }

        add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_opcode(chunk, OP_LOAD, resolve_variable("___size___"));
        add_code(chunk, OP_GT_EQ, node->line);
        add_opcode(chunk, OP_POP_JUMP_IF_TRUE, 5, node->line);

        add_opcode(chunk, OP_JUMP_BACK, chunk.code.size() - loop_start + 4, node->line);

        end_scope(chunk);
    }
}

void gen_accessor(Chunk& chunk, node_ptr node) {
    generate(node->_Node.Accessor().container, chunk);
    generate(node->_Node.Accessor().accessor->_Node.List().elements[0], chunk);
    add_code(chunk, OP_ACCESSOR, node->line);
}

void gen_break(Chunk& chunk, node_ptr node) {
    if (!current->in_loop) {
        error("Cannot use 'break' outside of a loop");
    }

    add_code(chunk, OP_BREAK, node->line);
}

void gen_continue(Chunk& chunk, node_ptr node) {
    if (!current->in_loop) {
        error("Cannot use 'continue' outside of a loop");
    }

    add_code(chunk, OP_CONTINUE, node->line);
}

void gen_return(Chunk& chunk, node_ptr node) {
    if (!current->in_function) {
        error("Cannot use 'return' at the top level");
    }

    if (node->_Node.Return().value) {
        generate(node->_Node.Return().value, chunk);
    } else {
        add_constant_code(chunk, none_val(), node->line);
    }
    add_code(chunk, OP_RETURN, node->line);
}

void gen_function(Chunk& chunk, node_ptr node) {

    auto prev_compiler = current;
    current = std::make_shared<Compiler>();
    current->prev = prev_compiler;

    current->in_function = true;

    std::shared_ptr<FunctionObj> function = std::make_shared<FunctionObj>();
    function->name = node->_Node.Function().name;
    function->arity = node->_Node.Function().params.size();
    function->chunk = Chunk();

    Value function_value = function_val();
    function_value.value = function;

    for (auto& param : node->_Node.Function().params) {
        std::string param_name = param->_Node.ID().value;
        if (node->_Node.Function().default_values.count(param_name)) {
            function->defaults++;
            generate(node->_Node.Function().default_values[param_name], function->chunk);
        } else {
            add_constant_code(function->chunk, none_val(), param->line);
        }

        declareVariable(param->_Node.ID().value);
    }

    add_constant_code(function->chunk, function_value, node->line);
    declareVariable(function->name);

    if (node->_Node.Function().body->type == NodeType::OBJECT) {
        generate_bytecode(node->_Node.Function().body->_Node.Object().elements, function->chunk);
        add_constant_code(function->chunk, none_val(), node->line);
        add_code(function->chunk, OP_RETURN, node->line);
    } else {
        generate(node->_Node.Function().body, function->chunk);
        add_code(function->chunk, OP_RETURN, node->line);
    }

    current->in_function = false;
    function->closed_vars = current->closed_vars;

    current = prev_compiler;
    add_constant_code(chunk, function_value, node->line);

    disassemble_chunk(function->chunk, function->name);
}

void gen_function_call(Chunk& chunk, node_ptr node) {
    for (int i = node->_Node.FunctionCall().args.size() - 1; i >= 0; i--) {
        generate(node->_Node.FunctionCall().args[i], chunk);
    }
    int index = resolve_variable(node->_Node.FunctionCall().name);
    if (index == -1) {
        add_constant_code(chunk, string_val(node->_Node.FunctionCall().name), node->line);
        add_code(chunk, OP_LOAD_GLOBAL, node->line);
    } else {
        add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.FunctionCall().name), node->line);
    }
    add_opcode(chunk, OP_CALL, node->_Node.FunctionCall().args.size(), node->line);
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
        case NodeType::LIST: {
            gen_list(chunk, node);
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
        case NodeType::FOR_LOOP: {
            gen_for_loop(chunk, node);
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
        case NodeType::RETURN: {
            gen_return(chunk, node);
            break;
        }
        case NodeType::ACCESSOR: {
            gen_accessor(chunk, node);
            break;
        }
        case NodeType::FUNC: {
            gen_function(chunk, node);
            break;
        }
        case NodeType::FUNC_CALL: {
            gen_function_call(chunk, node);
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
        if (node->_Node.Op().value == "..") {
            gen_range(chunk, node);
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
        if (node->type == NodeType::FUNC_CALL) {
            generate(node, chunk);
            add_code(chunk, OP_POP, node->line);
            continue;
        }
        generate(node, chunk);
    }
}

static void begin_scope() {
    current->scopeDepth++;
}

static void end_scope(Chunk& chunk) {
    current->scopeDepth--;

    while (current->variableCount > 0 
    && current->variables[current->variableCount - 1].depth > current->scopeDepth) {
        add_code(chunk, OP_POP);
        current->variables.erase(current->variables.begin() + current->variableCount - 1);
        current->variableCount--;
  }
}

static void addVariable(std::string name, bool is_const) {
    current->variableCount++;
    Variable variable;
    variable.name = name;
    variable.depth = current->scopeDepth;
    variable.is_const = is_const;
    current->variables.push_back(variable);
}

static void declareVariable(std::string name, bool is_const) {

    for (int i = current->variableCount - 1; i >= 0; i--) {
        Variable variable = current->variables[i];
        if (variable.depth != -1 && variable.depth < current->scopeDepth) {
            break; 
        }

        if (name == variable.name) {
            error("Variable '" + name + "' already defined");
        }
    }

    addVariable(name, is_const);
}

static int resolve_variable(std::string name) {
    for (int i = current->variableCount - 1; i >= 0; i--) {
        Variable variable = current->variables[i];
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