#pragma once
#include "../Interpreter.hpp"

node_ptr Interpreter::eval_try_catch(node_ptr& node) {
    global_interpreter->try_catch++;
    std::string error = "";
    for (node_ptr& expr : node->_Node.TryCatch().try_body->_Node.Object().elements) {
        node_ptr evaluated_expr = eval_node(expr);
        if (evaluated_expr->type == NodeType::_ERROR) {
            error = evaluated_expr->_Node.Error().message;
            break;
        }
        if (global_interpreter->error != "") {
            error = global_interpreter->error;
            break;
        }
        if (evaluated_expr->type == NodeType::RETURN) {
            return evaluated_expr;
        }
    }
    if (error != "") {
        global_interpreter->try_catch--;
        global_interpreter->error = "";
        symt_ptr catch_sym_table = std::make_shared<SymbolTable>();
        catch_sym_table->parent = current_scope;
        node_ptr error_arg = node->_Node.TryCatch().catch_keyword->_Node.FunctionCall().args[0];
        node_ptr error_node = new_string_node(error);
        add_symbol(error_arg->_Node.ID().value, error_node, catch_sym_table);

        current_scope = catch_sym_table;

        for (node_ptr& expr : node->_Node.TryCatch().catch_body->_Node.Object().elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                return evaluated_expr;
            }
        }

        current_scope = current_scope->parent;
    }

    return new_node(NodeType::NONE);
}