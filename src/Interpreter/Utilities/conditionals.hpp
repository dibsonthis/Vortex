#pragma once
#include "../Interpreter.hpp"

node_ptr Interpreter::eval_if_statement(node_ptr& node) {
    node_ptr conditional = eval_node(node->_Node.IfStatement().condition);
    bool is_true = false;

    if (conditional->type == NodeType::BOOLEAN) {
        is_true = conditional->_Node.Boolean().value;
    } else {
        is_true = conditional->type != NodeType::NONE;
    }

    if (is_true) {

        auto scope = std::make_shared<SymbolTable>();
        scope->parent = current_scope;
        current_scope = scope;

        for (node_ptr expr : node->_Node.IfStatement().body->_Node.Object().elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                node_ptr ret_clone = new_node(NodeType::RETURN);
                ret_clone->_Node.Return().value = eval_node(evaluated_expr->_Node.Return().value);
                current_scope = current_scope->parent;
                return ret_clone;
            }
            if (evaluated_expr->type == NodeType::BREAK) {
                current_scope = current_scope->parent;
                return evaluated_expr;
            }
            if (evaluated_expr->type == NodeType::CONTINUE) {
                current_scope = current_scope->parent;
                return evaluated_expr;
            }
        }

        current_scope = current_scope->parent;
    }

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_if_block(node_ptr& node) {
    for (node_ptr statement : node->_Node.IfBlock().statements) {
        if (statement->type == NodeType::IF_STATEMENT) {
            node_ptr conditional = eval_node(statement->_Node.IfStatement().condition);
            
            bool is_true = false;

            if (conditional->type == NodeType::BOOLEAN) {
                is_true = conditional->_Node.Boolean().value;
            } else {
                is_true = conditional->type != NodeType::NONE;
            }

            if (is_true) {
                return eval_node(statement);
            }
        } else if (statement->type == NodeType::OBJECT) {

            auto scope = std::make_shared<SymbolTable>();
            scope->parent = current_scope;
            current_scope = scope;

            for (node_ptr expr : statement->_Node.Object().elements) {
                if (expr->type == NodeType::RETURN) {
                    current_scope = current_scope->parent;
                    return eval_node(expr);
                }
                if (expr->type == NodeType::BREAK) {
                    current_scope = current_scope->parent;
                    return eval_node(expr);
                }
                if (expr->type == NodeType::CONTINUE) {
                    current_scope = current_scope->parent;
                    return eval_node(expr);
                }
                eval_node(expr);
            }
            current_scope = current_scope->parent;
            return new_node(NodeType::NONE);
        }
    }

    return new_node(NodeType::NONE);
}