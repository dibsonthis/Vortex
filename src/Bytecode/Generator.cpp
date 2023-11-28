#include "Generator.hpp"
#include "Bytecode.hpp"
#include "../Node/Node.hpp"

std::shared_ptr<Compiler> current = std::make_shared<Compiler>();

void gen_literal(Chunk &chunk, node_ptr node)
{
    switch (node->type)
    {
    case NodeType::NUMBER:
    {
        add_constant_code(chunk, number_val(node->_Node.Number().value), node->line);
        break;
    }
    case NodeType::STRING:
    {
        add_constant_code(chunk, string_val(node->_Node.String().value), node->line);
        break;
    }
    case NodeType::BOOLEAN:
    {
        add_constant_code(chunk, boolean_val(node->_Node.Boolean().value), node->line);
        break;
    }
    case NodeType::NONE:
    {
        add_constant_code(chunk, none_val(), node->line);
        break;
    }
    default:
    {
        return;
    }
    }
}

void gen_paren(Chunk &chunk, node_ptr node)
{
    if (node->_Node.Paren().elements.size() != 0)
    {
        generate(node->_Node.Paren().elements[0], chunk);
    }
}

void gen_list(Chunk &chunk, node_ptr node)
{
    if (node->_Node.List().elements.size() == 1 && node->_Node.List().elements[0]->type == NodeType::COMMA_LIST)
    {
        node = node->_Node.List().elements[0];
    }

    for (node_ptr &elem : node->_Node.List().elements)
    {
        generate(elem, chunk);
    }
    add_opcode(chunk, OP_BUILD_LIST, node->_Node.List().elements.size(), node->line);
}

void gen_neg(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_NEGATE, node->line);
}

void gen_add(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_ADD, node->line);
}

void gen_multiply(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_MULTIPLY, node->line);
}

void gen_subtract(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_SUBTRACT, node->line);
}

void gen_divide(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_DIVIDE, node->line);
}

void gen_mod(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_MOD, node->line);
}

void gen_pow(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_POW, node->line);
}

void gen_bin_and(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_AND, node->line);
}

void gen_bin_or(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_OR, node->line);
}

void gen_not(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_NOT, node->line);
}

void gen_eq_eq(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_EQ_EQ, node->line);
}

void gen_not_eq(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_NOT_EQ, node->line);
}

void gen_lt_eq(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_LT_EQ, node->line);
}

void gen_gt_eq(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_GT_EQ, node->line);
}

void gen_lt(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_LT, node->line);
}

void gen_gt(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_GT, node->line);
}

void gen_eq(Chunk &chunk, node_ptr node)
{
    node_ptr left = node->_Node.Op().left;
    if (left->type == NodeType::ID)
    {
        // int index = resolve_variable(left->_Node.ID().value);
        // int closure_index = -1;
        // if (index == -1) {
        //     closure_index = resolve_closure(left->_Node.ID().value);
        //     if (closure_index == -1) {
        //         error("Variable '" + left->_Node.ID().value + "' is undefined");
        //     }
        // }
        // Variable variable = current->variables[index];

        generate(node->_Node.Op().right, chunk);

        int index = resolve_variable(left->_Node.ID().value);

        if (index != -1)
        {
            Variable variable = current->variables[index];
            if (variable.is_const)
            {
                error("Cannot modify constant '" + left->_Node.ID().value + "'");
            }
            add_opcode(chunk, OP_SET, index, node->line);
            return;
        }
        index = resolve_closure_nested(left->_Node.ID().value);
        if (index == -1)
        {
            error("Variable '" + left->_Node.ID().value + "' is undefined");
        }
        else
        {
            add_opcode(chunk, OP_SET_CLOSURE, index, node->line);
        }
        // if (index >= 0 && variable.is_const) {
        //     error("Cannot modify constant '" + left->_Node.ID().value + "'");
        // }
        // generate(node->_Node.Op().right, chunk);
        // if (closure_index != -1) {
        //     bool captured = false;
        //     for (int var : current->closed_vars) {
        //         if (index == var) {
        //             captured = true;
        //             break;
        //         }
        //     }
        //     if (!captured) {
        //         current->closed_vars.push_back(closure_index);
        //     }
        //     add_opcode(chunk, OP_SET_CLOSURE, closure_index, node->line);
        // } else {
        //     add_opcode(chunk, OP_SET, index, node->line);
        // }
    }
    else if (left->type == NodeType::ACCESSOR)
    {
        if (left->_Node.Accessor().container->type == NodeType::ID)
        {
            int index = resolve_variable(left->_Node.Accessor().container->_Node.ID().value);
            if (index != -1)
            {
                Variable variable = current->variables[index];
                if (variable.is_const)
                {
                    error("Cannot modify constant '" + variable.name + "'");
                }
            }
        }
        generate(left->_Node.Accessor().container, chunk);
        generate(left->_Node.Accessor().accessor->_Node.List().elements[0], chunk);
        generate(node->_Node.Op().right, chunk);
        add_code(chunk, OP_SET_PROPERTY, node->line);
    }
    else if (left->type == NodeType::OP && left->_Node.Op().value == ".")
    {
        if (left->_Node.Op().left->type == NodeType::ID)
        {
            int index = resolve_variable(left->_Node.Op().left->_Node.ID().value);
            if (index != -1)
            {
                Variable variable = current->variables[index];
                if (variable.is_const)
                {
                    error("Cannot modify constant '" + variable.name + "'");
                }
            }
        }
        if (left->_Node.Op().right->type == NodeType::ID)
        {
            generate(left->_Node.Op().left, chunk);
            add_constant_code(chunk, string_val(left->_Node.Op().right->_Node.ID().value), node->line);
            generate(node->_Node.Op().right, chunk);
            add_code(chunk, OP_SET_PROPERTY, node->line);
            return;
        }

        error("Right hand side of '.' operator must be an identifier");
    }
}

void gen_range(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    generate(node->_Node.Op().right, chunk);
    add_code(chunk, OP_RANGE, node->line);
}

void gen_var(Chunk &chunk, node_ptr node)
{
    node_ptr &value = node->_Node.VariableDeclaration().value;
    generate(value, chunk);
    if (value->type == NodeType::FUNC)
    {
        for (auto &decorator : value->Meta.decorators)
        {
            if (decorator->type == NodeType::FUNC_CALL)
            {
                for (int i = decorator->_Node.FunctionCall().args.size() - 1; i >= 0; i--)
                {
                    node_ptr &arg = decorator->_Node.FunctionCall().args[i];
                    if (arg->type == NodeType::OP && arg->_Node.Op().value == "...")
                    {
                        generate(arg->_Node.Op().right, chunk);
                        add_code(chunk, OP_UNPACK, decorator->line);
                    }
                    else
                    {
                        generate(arg, chunk);
                    }
                }
                node_ptr id = std::make_shared<Node>(NodeType::ID);
                id->_Node.ID().value = decorator->_Node.FunctionCall().name;
                gen_id(chunk, id);
                // Magic number 2: the function + initial arg we're pushing
                add_opcode(chunk, OP_REMOVE_PUSH, decorator->_Node.FunctionCall().args.size() + 2, decorator->line);
                add_code(chunk, OP_SWAP_TOS, decorator->line);
                add_opcode(chunk, OP_CALL, decorator->_Node.FunctionCall().args.size() + 1, decorator->line);
            }
            else
            {
                generate(decorator, chunk);
                add_opcode(chunk, OP_CALL, 1, node->line);
            }
        }
    }
    declareVariable(node->_Node.VariableDeclaration().name);
    add_code(chunk, OP_MAKE_NON_CONST, node->line);
}

void gen_const(Chunk &chunk, node_ptr node)
{
    node_ptr &value = node->_Node.ConstantDeclatation().value;
    generate(value, chunk);
    if (value->type == NodeType::FUNC)
    {
        for (auto &decorator : value->Meta.decorators)
        {
            if (decorator->type == NodeType::FUNC_CALL)
            {
                for (int i = decorator->_Node.FunctionCall().args.size() - 1; i >= 0; i--)
                {
                    node_ptr &arg = decorator->_Node.FunctionCall().args[i];
                    if (arg->type == NodeType::OP && arg->_Node.Op().value == "...")
                    {
                        generate(arg->_Node.Op().right, chunk);
                        add_code(chunk, OP_UNPACK, decorator->line);
                    }
                    else
                    {
                        generate(arg, chunk);
                    }
                }
                node_ptr id = std::make_shared<Node>(NodeType::ID);
                id->_Node.ID().value = decorator->_Node.FunctionCall().name;
                gen_id(chunk, id);
                // Magic number 2: the function + initial arg we're pushing
                add_opcode(chunk, OP_REMOVE_PUSH, decorator->_Node.FunctionCall().args.size() + 2, decorator->line);
                add_code(chunk, OP_SWAP_TOS, decorator->line);
                add_opcode(chunk, OP_CALL, decorator->_Node.FunctionCall().args.size() + 1, decorator->line);
            }
            else
            {
                generate(decorator, chunk);
                add_opcode(chunk, OP_CALL, 1, node->line);
            }
        }
    }
    declareVariable(node->_Node.ConstantDeclatation().name, true);
    add_code(chunk, OP_MAKE_CONST, node->line);
}

void gen_id(Chunk &chunk, node_ptr node, int global_flag)
{
    if (node->_Node.ID().value == "this")
    {
        // if (!current->in_object) {
        //     error("Cannot use 'this' in outer scope");
        // }
        if (current->nested_object_count == 0)
        {
            error("Cannot use 'this' in outer scope");
        }
        add_code(chunk, OP_LOAD_THIS, node->line);
        return;
    }
    int index = resolve_variable(node->_Node.ID().value);
    if (index != -1)
    {
        add_opcode(chunk, OP_LOAD, index, node->line);
        return;
    }
    index = resolve_closure_nested(node->_Node.ID().value);
    if (index == -1)
    {
        add_constant_code(chunk, string_val(node->_Node.ID().value), node->line);
        add_opcode(chunk, OP_LOAD_GLOBAL, global_flag, node->line);
    }
    else
    {
        add_opcode(chunk, OP_LOAD_CLOSURE, index, node->line);
    }
    // int index = resolve_variable(node->_Node.ID().value);
    //  if (index == -1) {
    //      auto prev_compiler = current;
    //      if (!current->prev) {
    //          goto is_global;
    //      }
    //      current = current->prev;
    //      int index = resolve_variable(node->_Node.ID().value);
    //      if (index == -1) {
    //          current = prev_compiler;
    //          goto is_global;
    //      }
    //      bool captured = false;
    //      for (int var : prev_compiler->closed_vars) {
    //          if (index == var) {
    //              captured = true;
    //              break;
    //          }
    //      }
    //      if (!captured) {
    //          prev_compiler->closed_vars.push_back(index);
    //      }
    //      add_opcode(chunk, OP_LOAD_CLOSURE, index, node->line);
    //      current = prev_compiler;
    //      return;
    //  }
    //  add_opcode(chunk, OP_LOAD, index, node->line);
    //  return;

    // is_global:
    //     add_constant_code(chunk, string_val(node->_Node.ID().value), node->line);
    //     add_opcode(chunk, OP_LOAD_GLOBAL, global_flag, node->line);
    //     return;
}

int gen_if(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.IfStatement().condition, chunk);
    begin_scope();
    int jump_instruction = chunk.code.size() + 1;
    add_opcode(chunk, OP_POP_JUMP_IF_FALSE, 0, node->line);
    generate_bytecode(node->_Node.IfStatement().body->_Node.Object().elements, chunk);
    end_scope(chunk);
    int jump_true_instruction = chunk.code.size() + 1;
    add_opcode(chunk, OP_JUMP, 0, node->line);
    int offset = chunk.code.size() - jump_instruction - 4;
    uint8_t *bytes = int_to_bytes(offset);
    patch_bytes(chunk, jump_instruction, bytes);
    return jump_true_instruction;
}

void gen_if_block(Chunk &chunk, node_ptr node)
{
    std::vector<int> jump_instructions;
    int jump_instruction = gen_if(chunk, node->_Node.IfBlock().statements[0]);
    jump_instructions.push_back(jump_instruction);
    for (int i = 1; i < node->_Node.IfBlock().statements.size(); i++)
    {
        node_ptr &statement = node->_Node.IfBlock().statements[i];
        if (statement->type == NodeType::IF_STATEMENT)
        {
            int jump_instr = gen_if(chunk, statement);
            jump_instructions.push_back(jump_instr);
        }
        else if (statement->type == NodeType::OBJECT)
        {
            begin_scope();
            generate_bytecode(statement->_Node.Object().elements, chunk);
            end_scope(chunk);
        }
    }

    for (int jump_instruction : jump_instructions)
    {
        int offset = chunk.code.size() - jump_instruction - 4;
        uint8_t *bytes = int_to_bytes(offset);
        patch_bytes(chunk, jump_instruction, bytes);
    }
}

void gen_while_loop(Chunk &chunk, node_ptr node)
{
    add_opcode(chunk, OP_LOOP, 0, node->line);
    int start_index = chunk.code.size() - 1;
    generate(node->_Node.WhileLoop().condition, chunk);
    int jump_instruction = chunk.code.size() + 1;
    add_opcode(chunk, OP_POP_JUMP_IF_FALSE, 0, node->line);
    begin_scope();
    current->in_loop = true;
    generate_bytecode(node->_Node.WhileLoop().body->_Node.Object().elements, chunk);
    end_scope(chunk);
    current->in_loop = false;
    add_opcode(chunk, OP_JUMP_BACK, chunk.code.size() - start_index + 4, node->line);
    int offset = chunk.code.size() - jump_instruction - 4;
    uint8_t *bytes = int_to_bytes(offset);
    patch_bytes(chunk, jump_instruction, bytes);
    add_code(chunk, OP_LOOP_END, node->line);
}

void gen_for_loop(Chunk &chunk, node_ptr node)
{
    current->in_loop = true;
    if (!node->_Node.ForLoop().iterator)
    {
        if (!node->_Node.ForLoop().index_name)
        {
            node->_Node.ForLoop().index_name = std::make_shared<Node>(NodeType::ID);
            node->_Node.ForLoop().index_name->_Node.ID().value = "___index___";
        }

        begin_scope();

        add_constant_code(chunk, number_val(0));
        declareVariable(node->_Node.ForLoop().index_name->_Node.ID().value, false, true);

        if (node->_Node.ForLoop().value_name)
        {
            generate(node->_Node.ForLoop().start, chunk);
            declareVariable(node->_Node.ForLoop().value_name->_Node.ID().value, false, true);
        }

        generate(node->_Node.ForLoop().end, chunk);
        generate(node->_Node.ForLoop().start, chunk);
        add_code(chunk, OP_SUBTRACT, node->line);
        declareVariable("___size___", false, true);

        int loop_start = chunk.code.size() - 1;

        add_opcode(chunk, OP_LOOP, 0, node->line);

        begin_scope();

        generate_bytecode(node->_Node.ForLoop().body->_Node.Object().elements, chunk);

        end_scope(chunk);

        add_code(chunk, OP_ITER, node->line);
        add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_constant_code(chunk, number_val(1), node->line);
        add_code(chunk, OP_ADD, node->line);
        add_opcode(chunk, OP_SET_FORCE, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_code(chunk, OP_POP, node->line);

        if (node->_Node.ForLoop().value_name)
        {
            add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().value_name->_Node.ID().value), node->line);
            add_constant_code(chunk, number_val(1), node->line);
            add_code(chunk, OP_ADD, node->line);
            add_opcode(chunk, OP_SET_FORCE, resolve_variable(node->_Node.ForLoop().value_name->_Node.ID().value), node->line);
            add_code(chunk, OP_POP, node->line);
        }

        add_code(chunk, OP_ITER, node->line);
        add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_opcode(chunk, OP_LOAD, resolve_variable("___size___"));
        add_code(chunk, OP_GT_EQ, node->line);
        add_opcode(chunk, OP_POP_JUMP_IF_TRUE, 5, node->line);

        add_opcode(chunk, OP_JUMP_BACK, chunk.code.size() - loop_start + 4, node->line);

        end_scope(chunk);
    }
    else
    {
        if (!node->_Node.ForLoop().index_name)
        {
            node->_Node.ForLoop().index_name = std::make_shared<Node>(NodeType::ID);
            node->_Node.ForLoop().index_name->_Node.ID().value = "___index___";
        }

        begin_scope();

        add_constant_code(chunk, number_val(0));
        declareVariable(node->_Node.ForLoop().index_name->_Node.ID().value, false, true);

        generate(node->_Node.ForLoop().iterator, chunk);
        declareVariable("___iter___", false, true);

        if (node->_Node.ForLoop().value_name)
        {
            add_opcode(chunk, OP_LOAD, resolve_variable("___iter___"), node->line);
            add_constant_code(chunk, number_val(0));
            add_opcode(chunk, OP_ACCESSOR, 0, node->line);
            declareVariable(node->_Node.ForLoop().value_name->_Node.ID().value, false, true);
        }

        add_opcode(chunk, OP_LOAD, resolve_variable("___iter___"), node->line);
        add_code(chunk, OP_LEN, node->line);
        declareVariable("___size___", false, true);

        add_opcode(chunk, OP_LOAD, resolve_variable("___size___"));
        add_constant_code(chunk, number_val(0), node->line);
        add_code(chunk, OP_EQ_EQ, node->line);

        int jump_if_empty = chunk.code.size() + 1;
        add_opcode(chunk, OP_POP_JUMP_IF_TRUE, 0, node->line);

        int loop_start = chunk.code.size() - 1;

        add_opcode(chunk, OP_LOOP, 0, node->line);

        begin_scope();

        generate_bytecode(node->_Node.ForLoop().body->_Node.Object().elements, chunk);

        end_scope(chunk);

        add_code(chunk, OP_ITER, node->line);
        add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_constant_code(chunk, number_val(1), node->line);
        add_code(chunk, OP_ADD, node->line);
        add_opcode(chunk, OP_SET_FORCE, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_code(chunk, OP_POP, node->line);

        if (node->_Node.ForLoop().value_name)
        {
            add_opcode(chunk, OP_LOAD, resolve_variable("___iter___"), node->line);
            add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
            add_opcode(chunk, OP_ACCESSOR, 0, node->line);
            add_opcode(chunk, OP_SET_FORCE, resolve_variable(node->_Node.ForLoop().value_name->_Node.ID().value), node->line);
            add_code(chunk, OP_POP, node->line);
        }

        add_opcode(chunk, OP_LOAD, resolve_variable(node->_Node.ForLoop().index_name->_Node.ID().value), node->line);
        add_opcode(chunk, OP_LOAD, resolve_variable("___size___"));
        add_code(chunk, OP_GT_EQ, node->line);
        add_opcode(chunk, OP_POP_JUMP_IF_TRUE, 5, node->line);

        int offset = chunk.code.size() - jump_if_empty + 1;
        uint8_t *bytes = int_to_bytes(offset);
        patch_bytes(chunk, jump_if_empty, bytes);

        add_opcode(chunk, OP_JUMP_BACK, chunk.code.size() - loop_start + 4, node->line);

        end_scope(chunk);
    }
    add_code(chunk, OP_LOOP_END, node->line);
    current->in_loop = false;
}

void gen_accessor(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Accessor().container, chunk);
    generate(node->_Node.Accessor().accessor->_Node.List().elements[0], chunk);
    add_opcode(chunk, OP_ACCESSOR, 0, node->line);
}

void gen_dot(Chunk &chunk, node_ptr node)
{
    if (node->_Node.Op().right->type == NodeType::ID)
    {
        generate(node->_Node.Op().left, chunk);
        add_constant_code(chunk, string_val(node->_Node.Op().right->_Node.ID().value), node->line);
    }
    else if (node->_Node.Op().right->type == NodeType::FUNC_CALL)
    {
        for (int i = node->_Node.Op().right->_Node.FunctionCall().args.size() - 1; i >= 0; i--)
        {
            // generate(node->_Node.Op().right->_Node.FunctionCall().args[i], chunk);
            node_ptr &arg = node->_Node.Op().right->_Node.FunctionCall().args[i];
            if (arg->type == NodeType::OP && arg->_Node.Op().value == "...")
            {
                generate(arg->_Node.Op().right, chunk);
                add_code(chunk, OP_UNPACK, arg->line);
            }
            else
            {
                generate(arg, chunk);
            }
        }
        generate(node->_Node.Op().left, chunk);
        add_constant_code(chunk, string_val(node->_Node.Op().right->_Node.FunctionCall().name), node->line);
        add_opcode(chunk, OP_ACCESSOR, 1, node->line);
        node_ptr backup_function_node = std::make_shared<Node>(NodeType::ID);
        backup_function_node->_Node.ID().value = node->_Node.Op().right->_Node.FunctionCall().name;
        gen_id(chunk, backup_function_node, 1);
        add_opcode(chunk, OP_CALL_METHOD, node->_Node.Op().right->_Node.FunctionCall().args.size(), node->line);
        return;
    }
    else if (node->_Node.Op().right->type == NodeType::ACCESSOR)
    {
        node_ptr new_dot = std::make_shared<Node>(NodeType::OP);
        new_dot->_Node.Op().value = ".";
        new_dot->_Node.Op().left = node->_Node.Op().left;
        new_dot->_Node.Op().right = node->_Node.Op().right->_Node.Accessor().container;
        gen_dot(chunk, new_dot);
        generate(node->_Node.Op().right->_Node.Accessor().accessor->_Node.List().elements[0], chunk);
    }
    else
    {
        generate(node->_Node.Op().left, chunk);
        generate(node->_Node.Op().right, chunk);
    }
    add_opcode(chunk, OP_ACCESSOR, 0, node->line);
}

void gen_hook(Chunk &chunk, node_ptr node)
{
    if (node->_Node.Op().left->type == NodeType::ID)
    {
        int index = resolve_variable(node->_Node.Op().left->_Node.ID().value);
        if (index == -1)
        {
            error("Cannot assign hook to closure or global variable");
        }

        if (node->_Node.Op().right->type != NodeType::FUNC_CALL)
        {
            error("Hook must be a function call");
        }

        if (node->_Node.Op().right->_Node.FunctionCall().args.size() != 1)
        {
            error("Cannot assign empty hook - make sure to include a valid function");
        }

        std::string hook_name = node->_Node.Op().right->_Node.FunctionCall().name;
        node_ptr function = node->_Node.Op().right->_Node.FunctionCall().args[0];

        generate(function, chunk);
        if (hook_name == "onChange")
        {
            add_opcode(chunk, OP_HOOK_ONCHANGE, index, node->line);
        }
        else
        {
            error("Invalid hook name: '" + hook_name + "'");
        }
    }
    else
    {
        error("Left hand side of hook assignment must be an identifier");
    }
}

void gen_break(Chunk &chunk, node_ptr node)
{
    if (!current->in_loop)
    {
        error("Cannot use 'break' outside of a loop");
    }

    add_code(chunk, OP_BREAK, node->line);
}

void gen_continue(Chunk &chunk, node_ptr node)
{
    if (!current->in_loop)
    {
        error("Cannot use 'continue' outside of a loop");
    }

    add_code(chunk, OP_CONTINUE, node->line);
}

void gen_return(Chunk &chunk, node_ptr node)
{
    if (!current->in_function)
    {
        error("Cannot use 'return' at the top level");
    }

    if (node->_Node.Return().value)
    {
        generate(node->_Node.Return().value, chunk);
    }
    else
    {
        add_constant_code(chunk, none_val(), node->line);
    }
    add_code(chunk, OP_RETURN, node->line);
}

void gen_yield(Chunk &chunk, node_ptr node)
{
    if (!current->in_function)
    {
        error("Cannot use 'yield' at the top level");
    }

    if (node->_Node.Yield().value)
    {
        generate(node->_Node.Yield().value, chunk);
    }
    else
    {
        add_constant_code(chunk, none_val(), node->line);
    }
    add_code(chunk, OP_YIELD, node->line);
}

void gen_function(Chunk &chunk, node_ptr node)
{

    auto prev_compiler = current;
    current = std::make_shared<Compiler>();
    current->name = node->_Node.Function().name;
    current->root = false;
    current->prev = prev_compiler;
    current->in_object = prev_compiler->in_object;
    current->nested_object_count = prev_compiler->nested_object_count;

    current->in_function = true;

    std::shared_ptr<FunctionObj> function = std::make_shared<FunctionObj>();
    function->name = node->_Node.Function().name;
    function->arity = node->_Node.Function().params.size();
    function->chunk = Chunk();
    function->chunk.import_path = chunk.import_path;

    Value function_value = function_val();
    function_value.value = function;

    function->is_generator = node->_Node.Function().is_generator;
    function->is_type_generator = node->_Node.Function().is_type_generator;

    for (auto &param : node->_Node.Function().params)
    {
        std::string param_name = param->_Node.ID().value;
        bool is_capture = param->Meta.tags.size() == 1 && param->Meta.tags[0] == "capture";
        if (is_capture)
        {
            node->_Node.Function().default_values[param_name] = std::make_shared<Node>(NodeType::LIST);
        }
        function->params.push_back(param_name);

        Value placeholder = list_val();
        placeholder.meta.packer = true;

        if (node->_Node.Function().default_values.count(param_name))
        {
            if (is_capture && node->_Node.Function().default_values[param_name]->type != NodeType::LIST)
            {
                error("Capture param (...) cannot have a default value");
            }
            function->defaults++;
            auto temp = current;
            current = prev_compiler;
            generate(node->_Node.Function().default_values[param_name], chunk);
            current = temp;
            add_constant_code(function->chunk, is_capture ? placeholder : none_val(), param->line);
        }
        else
        {
            is_capture ? add_constant_code(function->chunk, placeholder, param->line) : add_constant_code(function->chunk, none_val(), param->line);
        }

        declareVariable(param->_Node.ID().value);
    }

    add_constant_code(function->chunk, function_value, node->line);
    declareVariable(function->name);

    if (function->is_generator)
    {
        Value none = none_val();
        add_constant_code(function->chunk, none, node->line);
        declareVariable("_value", false);
    }

    if (node->_Node.Function().body->type == NodeType::OBJECT)
    {
        generate_bytecode(node->_Node.Function().body->_Node.Object().elements, function->chunk);
        add_constant_code(function->chunk, none_val(), node->line);
        add_code(function->chunk, OP_RETURN, node->line);
    }
    else
    {
        generate(node->_Node.Function().body, function->chunk);
        add_code(function->chunk, OP_RETURN, node->line);
    }

    current->in_function = false;
    function->closed_var_indexes = current->closed_vars;

    current = prev_compiler;
    add_constant_code(chunk, function_value, node->line);

    if (function->closed_var_indexes.size() > 0)
    {
        add_opcode(chunk, OP_MAKE_CLOSURE, 0, node->line);
    }

    if (function->defaults > 0)
    {
        add_opcode(chunk, OP_MAKE_FUNCTION, function->defaults, node->line);
    }

    auto offsets = instruction_offsets(function->chunk);
    function->instruction_offsets = offsets;

    // disassemble_chunk(function->chunk, function->name);
}

void gen_function_call(Chunk &chunk, node_ptr node)
{
    for (int i = node->_Node.FunctionCall().args.size() - 1; i >= 0; i--)
    {
        node_ptr &arg = node->_Node.FunctionCall().args[i];
        if (arg->type == NodeType::OP && arg->_Node.Op().value == "...")
        {
            generate(arg->_Node.Op().right, chunk);
            add_code(chunk, OP_UNPACK, node->line);
        }
        else
        {
            generate(arg, chunk);
        }
    }
    node_ptr id = std::make_shared<Node>(NodeType::ID);
    id->_Node.ID().value = node->_Node.FunctionCall().name;
    gen_id(chunk, id);
    add_opcode(chunk, OP_CALL, node->_Node.FunctionCall().args.size(), node->line);
}

void gen_type(Chunk &chunk, node_ptr node)
{
    TypeNode &type_node = node->_Node.Type();

    if (type_node.body && type_node.body->type == NodeType::FUNC)
    {
        // We just make this a const decl
        // But tag the function as a type generator
        node_ptr const_decl = std::make_shared<Node>(NodeType::CONSTANT_DECLARATION);
        const_decl->_Node.ConstantDeclatation().name = type_node.name;
        const_decl->_Node.ConstantDeclatation().value = type_node.body;
        const_decl->_Node.ConstantDeclatation().value->_Node.Function().is_type_generator = true;
        gen_const(chunk, const_decl);
        return;
    }

    std::vector<node_ptr> elements;
    add_constant_code(chunk, string_val(type_node.name), node->line);
    if (
        !type_node.body ||
        (type_node.body->type == NodeType::OBJECT && type_node.body->_Node.Object().elements.size() == 0))
    {
        add_opcode(chunk, OP_MAKE_TYPE, 0, node->line);
        return;
    }

    if (type_node.body->type != NodeType::OBJECT)
    {
        error("Invalid type body");
    }

    if (type_node.body->_Node.Object().elements.size() == 1 && type_node.body->_Node.Object().elements[0]->type == NodeType::COMMA_LIST)
    {
        elements = type_node.body->_Node.Object().elements[0]->_Node.List().elements;
    }
    else
    {
        elements.push_back(type_node.body->_Node.Object().elements[0]);
    }

    std::unordered_map<std::string, node_ptr> defaults;

    for (auto &elem : elements)
    {
        switch (elem->type)
        {
        case NodeType::ID:
        {
            add_constant_code(chunk, string_val(elem->_Node.ID().value), node->line);
            add_constant_code(chunk, type_val("None"), node->line);
            break;
        }
        case NodeType::OP:
        {
            node_ptr type_elem = elem;
            node_ptr default_elem;
            if (elem->_Node.Op().value == "=")
            {
                type_elem = elem->_Node.Op().left;
                default_elem = elem->_Node.Op().right;
                defaults[type_elem->_Node.Op().left->_Node.ID().value] = default_elem;
            }
            else
            {
                type_elem = elem;
            }

            add_constant_code(chunk, string_val(type_elem->_Node.Op().left->_Node.ID().value), node->line);
            // Due to how the parser is set up for built-in types, we need to do a conversion here
            NodeType right_type = type_elem->_Node.Op().right->type;
            switch (right_type)
            {
            case NodeType::STRING:
            {
                add_constant_code(chunk, type_val("String"), node->line);
                break;
            }
            case NodeType::NUMBER:
            {
                add_constant_code(chunk, type_val("Number"), node->line);
                break;
            }
            case NodeType::BOOLEAN:
            {
                add_constant_code(chunk, type_val("Boolean"), node->line);
                break;
            }
            case NodeType::LIST:
            {
                add_constant_code(chunk, type_val("List"), node->line);
                break;
            }
            case NodeType::OBJECT:
            {
                add_constant_code(chunk, type_val("Object"), node->line);
                break;
            }
            case NodeType::FUNC:
            {
                add_constant_code(chunk, type_val("Function"), node->line);
                break;
            }
            case NodeType::NONE:
            {
                add_constant_code(chunk, type_val("None"), node->line);
                break;
            }
            default:
            {
                generate(type_elem->_Node.Op().right, chunk);
                break;
            }
            }
        }
        default:
        {
            break;
        }
        }
    }

    add_opcode(chunk, OP_MAKE_TYPE, elements.size(), node->line);

    if (defaults.size() > 0)
    {
        for (auto &def : defaults)
        {
            add_constant_code(chunk, string_val(def.first), node->line);
            generate(def.second, chunk);
        }
        add_opcode(chunk, OP_TYPE_DEFAULTS, defaults.size(), node->line);
    }

    declareVariable(type_node.name, true);
}

void gen_typed_object(Chunk &chunk, node_ptr node)
{
    gen_object(chunk, node->_Node.ObjectDeconstruct().body);
    int index = resolve_variable(node->_Node.ObjectDeconstruct().name);
    add_opcode(chunk, OP_LOAD, index, node->line);
    add_code(chunk, OP_MAKE_TYPED, node->line);
    return;
}

void gen_object(Chunk &chunk, node_ptr node)
{
    // current->in_object = true;
    current->nested_object_count++;
    ObjectNode &object_node = node->_Node.Object();
    std::vector<node_ptr> elements;
    if (object_node.elements.size() == 0)
    {
        add_opcode(chunk, OP_MAKE_OBJECT, 0, node->line);
        return;
    }

    if (object_node.elements.size() == 1 && object_node.elements[0]->type == NodeType::COMMA_LIST)
    {
        elements = object_node.elements[0]->_Node.List().elements;
    }
    else
    {
        elements.push_back(object_node.elements[0]);
    }

    for (auto &elem : elements)
    {
        if (elem->type != NodeType::OP || elem->_Node.Op().value != ":")
        {
            error("Object properties must be in shape (name: value)");
        }
        if (elem->_Node.Op().left->type == NodeType::ID)
        {
            add_constant_code(chunk, string_val(elem->_Node.Op().left->_Node.ID().value), node->line);
        }
        else
        {
            generate(elem->_Node.Op().left, chunk);
        }
        generate(elem->_Node.Op().right, chunk);
    }

    add_opcode(chunk, OP_MAKE_OBJECT, elements.size(), node->line);
    // current->in_object = false;
    current->nested_object_count--;
}

void gen_import(Chunk &chunk, node_ptr node)
{
    if (node->_Node.Import().is_default)
    {
        node->_Node.Import().target = std::make_shared<Node>(NodeType::STRING);
        node->_Node.Import().target->_Node.String().value = "@modules/" + node->_Node.Import().module->_Node.ID().value;
    }

    if (node->_Node.Import().target->type == NodeType::ID)
    {
        std::string target_value = node->_Node.Import().target->_Node.ID().value;
        node->_Node.Import().target = std::make_shared<Node>(NodeType::STRING);
        node->_Node.Import().target->_Node.String().value = "@modules/" + target_value;
    }

    if (node->_Node.Import().target->type != NodeType::STRING)
    {
        error("Import target must be a string");
    }

    std::string target_name = std::string(node->_Node.Import().target->_Node.String().value);

    replaceAll(target_name, "@modules/", "");

    std::string path = node->_Node.Import().target->_Node.String().value + ".vtx";

#if GCC_COMPILER
#if __apple__ || __linux__
    if (chunk.import_path != "")
    {
        replaceAll(path, "@modules", chunk.import_path + "/" + target_name);
    }
    else
    {
        replaceAll(path, "@modules", "/usr/local/share/vortex/modules/" + target_name);
    }
#else
    if (chunk.import_path != "")
    {
        replaceAll(path, "@modules", chunk.import_path + "/" + target_name);
    }
    else
    {
        replaceAll(path, "@modules", "C:/Program Files/vortex/modules/" + target_name);
    }
#endif
#else
#if defined(__APPLE__) || defined(__linux__)
    if (chunk.import_path != "")
    {
        replaceAll(path, "@modules", chunk.import_path + "/" + target_name);
    }
    else
    {
        replaceAll(path, "@modules", "/usr/local/share/vortex/modules/" + target_name);
    }
#else
    if (chunk.import_path != "")
    {
        replaceAll(path, "@modules", chunk.import_path + "/" + target_name);
    }
    else
    {
        replaceAll(path, "@modules", "C:/Program Files/vortex/modules/" + target_name);
    }
#endif
#endif

    add_constant_code(chunk, string_val(path), node->line);

    if (node->_Node.Import().module->type == NodeType::ID)
    {
        // import x : x
        add_constant_code(chunk, string_val(node->_Node.Import().module->_Node.ID().value), node->line);
        declareVariable(node->_Node.Import().module->_Node.ID().value, true);
        add_opcode(chunk, OP_IMPORT, 0, node->line);
    }
    else if (node->_Node.Import().module->type == NodeType::LIST && node->_Node.Import().module->_Node.List().elements.size() == 0)
    {
        // import [] : x
        // if (current->in_function || current->in_object) {
        //     error("Cannot import [] outside of global scope");
        // }
        if (current->in_function || current->nested_object_count > 0)
        {
            error("Cannot import [] outside of global scope");
        }
        add_constant_code(chunk, none_val(), node->line);
        add_opcode(chunk, OP_IMPORT, 0, node->line);
    }
    else
    {
        // import [a, b, c] : x
        std::vector<node_ptr> elements;
        if (node->_Node.Import().module->_Node.List().elements.size() == 1 && node->_Node.Import().module->_Node.List().elements[0]->type == NodeType::COMMA_LIST)
        {
            elements = node->_Node.Import().module->_Node.List().elements[0]->_Node.List().elements;
        }
        else
        {
            elements = node->_Node.Import().module->_Node.List().elements;
        }
        for (auto &elem : elements)
        {
            if (elem->type != NodeType::ID)
            {
                error("Import variables must be identifiers");
            }
            add_constant_code(chunk, string_val(elem->_Node.ID().value), node->line);
            declareVariable(elem->_Node.ID().value, true);
        }
        add_opcode(chunk, OP_IMPORT, elements.size(), node->line);
    }
}

void gen_and(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    int jump_instruction = chunk.code.size() + 1;
    add_opcode(chunk, OP_JUMP_IF_FALSE, 0, node->line);
    add_code(chunk, OP_POP, node->line);
    generate(node->_Node.Op().right, chunk);
    int offset = chunk.code.size() - jump_instruction - 4;
    uint8_t *bytes = int_to_bytes(offset);
    patch_bytes(chunk, jump_instruction, bytes);
}

void gen_or(Chunk &chunk, node_ptr node)
{
    generate(node->_Node.Op().left, chunk);
    int jump_instruction = chunk.code.size() + 1;
    add_opcode(chunk, OP_JUMP_IF_TRUE, 0, node->line);
    add_code(chunk, OP_POP, node->line);
    generate(node->_Node.Op().right, chunk);
    int offset = chunk.code.size() - jump_instruction - 4;
    uint8_t *bytes = int_to_bytes(offset);
    patch_bytes(chunk, jump_instruction, bytes);
}

void generate(node_ptr node, Chunk &chunk)
{
    switch (node->type)
    {
    case NodeType::NUMBER:
    case NodeType::STRING:
    case NodeType::BOOLEAN:
    case NodeType::NONE:
    {
        gen_literal(chunk, node);
        break;
    }
    case NodeType::ID:
    {
        gen_id(chunk, node);
        break;
    }
    case NodeType::PAREN:
    {
        gen_paren(chunk, node);
        break;
    }
    case NodeType::LIST:
    {
        gen_list(chunk, node);
        break;
    }
    case NodeType::VARIABLE_DECLARATION:
    {
        gen_var(chunk, node);
        break;
    }
    case NodeType::CONSTANT_DECLARATION:
    {
        gen_const(chunk, node);
        break;
    }
    case NodeType::IF_STATEMENT:
    {
        gen_if(chunk, node);
        break;
    }
    case NodeType::IF_BLOCK:
    {
        gen_if_block(chunk, node);
        break;
    }
    case NodeType::WHILE_LOOP:
    {
        gen_while_loop(chunk, node);
        break;
    }
    case NodeType::FOR_LOOP:
    {
        gen_for_loop(chunk, node);
        break;
    }
    case NodeType::BREAK:
    {
        gen_break(chunk, node);
        break;
    }
    case NodeType::CONTINUE:
    {
        gen_continue(chunk, node);
        break;
    }
    case NodeType::RETURN:
    {
        gen_return(chunk, node);
        break;
    }
    case NodeType::YIELD:
    {
        gen_yield(chunk, node);
        break;
    }
    case NodeType::ACCESSOR:
    {
        gen_accessor(chunk, node);
        break;
    }
    case NodeType::FUNC:
    {
        gen_function(chunk, node);
        break;
    }
    case NodeType::FUNC_CALL:
    {
        gen_function_call(chunk, node);
        break;
    }
    case NodeType::TYPE:
    {
        gen_type(chunk, node);
        break;
    }
    case NodeType::OBJECT_DECONSTRUCT:
    {
        gen_typed_object(chunk, node);
        break;
    }
    case NodeType::OBJECT:
    {
        gen_object(chunk, node);
        break;
    }
    case NodeType::IMPORT:
    {
        gen_import(chunk, node);
        break;
    }
    case NodeType::PIPE_LIST:
    {
        for (int i = 0; i < node->_Node.List().elements.size(); i++)
        {
            node_ptr elem = node->_Node.List().elements[i];
            int size = node->_Node.List().elements.size() - i;
            if (size == 1)
            {
                generate(elem, chunk);
                add_code(chunk, OP_OR, node->line);
                break;
            }
            generate(elem, chunk);
            i++;
            generate(node->_Node.List().elements[i], chunk);
            add_code(chunk, OP_OR, node->line);
        }
        break;
    }
    default:
    {
        break;
    }
    }

    if (node->type == NodeType::OP)
    {
        if (node->_Node.Op().value == "-" && !node->_Node.Op().left)
        {
            gen_neg(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "+")
        {
            gen_add(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "*")
        {
            gen_multiply(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "-")
        {
            gen_subtract(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "/")
        {
            gen_divide(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "%")
        {
            gen_mod(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "^")
        {
            gen_pow(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "&")
        {
            gen_and(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "|")
        {
            gen_or(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "-=")
        {
            node_ptr sub_node = std::make_shared<Node>(NodeType::OP);
            sub_node->_Node.Op().value = "-";
            sub_node->_Node.Op().left = node->_Node.Op().left;
            sub_node->_Node.Op().right = node->_Node.Op().right;
            sub_node->line = node->line;

            node_ptr eq_node = std::make_shared<Node>(NodeType::OP);
            eq_node->_Node.Op().value = "=";
            eq_node->_Node.Op().left = node->_Node.Op().left;
            eq_node->_Node.Op().right = sub_node;
            eq_node->line = node->line;

            gen_eq(chunk, eq_node);
            return;
        }
        if (node->_Node.Op().value == "+=")
        {
            node_ptr add_node = std::make_shared<Node>(NodeType::OP);
            add_node->_Node.Op().value = "+";
            add_node->_Node.Op().left = node->_Node.Op().left;
            add_node->_Node.Op().right = node->_Node.Op().right;
            add_node->line = node->line;

            node_ptr eq_node = std::make_shared<Node>(NodeType::OP);
            eq_node->_Node.Op().value = "=";
            eq_node->_Node.Op().left = node->_Node.Op().left;
            eq_node->_Node.Op().right = add_node;
            eq_node->line = node->line;

            gen_eq(chunk, eq_node);
            return;
        }
        if (node->_Node.Op().value == "!")
        {
            gen_not(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "==")
        {
            gen_eq_eq(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "!=")
        {
            gen_not_eq(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "<=")
        {
            gen_lt_eq(chunk, node);
            return;
        }
        if (node->_Node.Op().value == ">=")
        {
            gen_gt_eq(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "<")
        {
            gen_lt(chunk, node);
            return;
        }
        if (node->_Node.Op().value == ">")
        {
            gen_gt(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "=")
        {
            gen_eq(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "and")
        {
            gen_and(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "or")
        {
            gen_or(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "..")
        {
            gen_range(chunk, node);
            return;
        }
        if (node->_Node.Op().value == ".")
        {
            gen_dot(chunk, node);
            return;
        }
        if (node->_Node.Op().value == "::")
        {
            gen_hook(chunk, node);
            return;
        }
        if (node->_Node.Op().value == ";")
        {
            return;
        }
        if (node->_Node.Op().value == "...")
        {
            error("Cannot unpack value here");
        }
    }
}

void generate_bytecode(std::vector<node_ptr> &nodes, Chunk &chunk)
{
    for (node_ptr &node : nodes)
    {
        if (node->type == NodeType::OP && node->_Node.Op().value == ";")
        {
            continue;
        }
        if (node->type == NodeType::OP && node->_Node.Op().value == "::")
        {
            generate(node, chunk);
            continue;
        }
        if (node->type == NodeType::OP)
        {
            generate(node, chunk);
            add_code(chunk, OP_POP, node->line);
            continue;
        }
        if (node->type == NodeType::FUNC_CALL ||
            node->type == NodeType::STRING ||
            node->type == NodeType::NUMBER ||
            node->type == NodeType::BOOLEAN ||
            node->type == NodeType::LIST ||
            node->type == NodeType::OBJECT)
        {
            generate(node, chunk);
            add_code(chunk, OP_POP, node->line);
            continue;
        }
        if (node->type == NodeType::OP && node->_Node.Op().value == ".")
        {
            generate(node, chunk);
            add_code(chunk, OP_POP, node->line);
            continue;
        }
        generate(node, chunk);
    }

    for (Variable &var : current->variables)
    {
        if (!var.is_internal)
        {
            if (std::find(chunk.public_variables.begin(), chunk.public_variables.end(), var.name) == chunk.public_variables.end())
            {
                chunk.public_variables.push_back(var.name);
            }
        }

        if (std::find(chunk.variables.begin(), chunk.variables.end(), var.name) == chunk.variables.end())
        {
            chunk.variables.push_back(var.name);
        }
    }
}

static void begin_scope()
{
    current->scopeDepth++;
}

static void end_scope(Chunk &chunk)
{
    current->scopeDepth--;

    while (current->variableCount > 0 && current->variables[current->variableCount - 1].depth > current->scopeDepth)
    {
        add_code(chunk, OP_POP);
        current->variables.erase(current->variables.begin() + current->variableCount - 1);
        // chunk.variables.pop_back();
        current->variableCount--;
    }
}

static void addVariable(std::string name, bool is_const, bool is_internal)
{
    current->variableCount++;
    Variable variable;
    variable.name = name;
    variable.depth = current->scopeDepth;
    variable.is_const = is_const;
    variable.is_internal = is_internal;

    current->variables.push_back(variable);
}

static void removeVariable(std::string name)
{
    current->variableCount--;
    for (int i = 0; i < current->variables.size(); i++)
    {
        if (current->variables[i].name == name)
        {
            current->variables.erase(current->variables.begin() + i);
        }
    }
}

static void declareVariable(std::string name, bool is_const, bool is_internal)
{

    for (int i = current->variableCount - 1; i >= 0; i--)
    {
        Variable variable = current->variables[i];
        if (variable.depth != -1 && variable.depth < current->scopeDepth)
        {
            break;
        }

        if (name == variable.name)
        {
            error("Variable '" + name + "' already defined");
        }
    }

    addVariable(name, is_const, is_internal);
}

static int resolve_variable(std::string name)
{
    for (int i = current->variableCount - 1; i >= 0; i--)
    {
        Variable variable = current->variables[i];
        if (name == variable.name)
        {
            return i;
        }
    }

    return -1;
}

static int resolve_closure(std::string name)
{
    auto prev_compiler = current;
    if (!current->prev)
    {
        return -1;
    }
    current = current->prev;
    int index = resolve_variable(name);
    if (index == -1)
    {
        current = prev_compiler;
        return -1;
    }
    current = prev_compiler;
    return index;
}

static int resolve_closure_nested(std::string name)
{
    int index = resolve_variable(name);
    if (index != -1)
    {
        return index;
    }

    std::vector<std::shared_ptr<Compiler>> compilers;
    auto original = current;
    auto _current = current;
    while (_current)
    {
        compilers.push_back(_current);
        _current = _current->prev;
    }

    bool is_local = true;

    for (int i = 1; i < compilers.size(); i++)
    {
        current = compilers[i];
        index = resolve_variable(name);
        if (index != -1)
        {
            if (i > 1)
            {
                is_local = false;
            }
            bool local = is_local;
            for (int j = i - 1; j >= 0; j--)
            {
                auto comp = compilers[j];
                if (compilers[j + 1]->root || (j == i - 1))
                {
                    is_local = true;
                }
                else
                {
                    is_local = local;
                }
                bool captured = false;
                for (int k = 0; k < compilers[j]->closed_vars.size(); k++)
                {
                    auto &var = compilers[j]->closed_vars[k];
                    if (is_local == var.is_local && name == var.name)
                    {
                        captured = true;
                        index = k;
                        break;
                    }
                }
                if (!captured)
                {
                    ClosedVar var;
                    var.name = name;
                    var.index = index;
                    index = compilers[j]->closed_vars.size();
                    var.is_local = is_local;
                    compilers[j]->closed_vars.push_back(var);
                }
            }
            current = original;
            return index;
        }
    }

    current = original;
    return -1;
}

void error(std::string message)
{
    std::cout << message << "\n";
    exit(1);
}

void reset()
{
    current = std::make_shared<Compiler>();
}