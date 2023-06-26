#pragma once
#include "../Interpreter.hpp"

node_ptr Interpreter::eval_for_loop(node_ptr& node) {

    if (node->_Node.ForLoop().iterator != nullptr) {
        node_ptr iterator = eval_node(node->_Node.ForLoop().iterator);

        // TODO: Implement for object looping

        if (iterator->type != NodeType::LIST) {
            return throw_error("For loop iterator must be a list");
        }

        auto curr_scope = current_scope;

        auto loop_symbol_table = std::make_shared<SymbolTable>();
        
        for (int i = 0; i < iterator->_Node.List().elements.size(); i++) {

            loop_symbol_table->symbols.clear();
            current_scope = loop_symbol_table;
            current_scope->parent = curr_scope;

            if (node->_Node.ForLoop().index_name) {
                node_ptr number_node = new_number_node(i);
                add_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, number_node, current_scope);
            }
            if (node->_Node.ForLoop().value_name) {
                add_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, iterator->_Node.List().elements[i], current_scope);
            }
            for (node_ptr expr : node->_Node.ForLoop().body->_Node.Object().elements) {
                node_ptr evaluated_expr = eval_node(expr);
                if (evaluated_expr->type == NodeType::RETURN) {
                    current_scope = curr_scope;
                    return evaluated_expr;
                }
                if (evaluated_expr->type == NodeType::BREAK) {
                    goto _break;
                }
                if (evaluated_expr->type == NodeType::CONTINUE) {
                    goto _continue;
                }
            }

            _continue:
                continue;
            _break:
                current_scope = curr_scope;
                break;
        }

        // Cleanup

        if (node->_Node.ForLoop().index_name != nullptr) {
            del_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, current_scope);
        }
        if (node->_Node.ForLoop().value_name != nullptr) {
            del_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, current_scope);
        }

        current_scope = curr_scope;

        return new_node(NodeType::NONE);
    }

    node_ptr start_node = eval_node(node->_Node.ForLoop().start);
    node_ptr end_node = eval_node(node->_Node.ForLoop().end);

    if (start_node->type != NodeType::NUMBER && end_node->type != NodeType::NUMBER) {
        return throw_error("For loop range expects two numbers");
    }

    int start = start_node->_Node.Number().value;
    int end = end_node->_Node.Number().value;

    auto curr_scope = current_scope;

    int for_index = -1;

    auto loop_symbol_table = std::make_shared<SymbolTable>();
    current_scope = loop_symbol_table;
    current_scope->parent = curr_scope;

    for (int i = start; i < end; i++) {

        for_index++;

        loop_symbol_table->symbols.clear();

        int index = i - start_node->_Node.Number().value;
        if (node->_Node.ForLoop().index_name) {
            node_ptr index_node = new_number_node(for_index);
            current_scope->symbols[node->_Node.ForLoop().index_name->_Node.ID().value] = index_node;
        }
        if (node->_Node.ForLoop().value_name) {
            node_ptr value_node = new_number_node(i);
            current_scope->symbols[node->_Node.ForLoop().value_name->_Node.ID().value] = value_node;
        }
        for (node_ptr& expr : node->_Node.ForLoop().body->_Node.Object().elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                current_scope = curr_scope;
                return evaluated_expr;
            }
            if (evaluated_expr->type == NodeType::BREAK) {
                goto _break2;
            }
            if (evaluated_expr->type == NodeType::CONTINUE) {
                goto _continue2;
            }
        }

        _continue2:
            continue;
        _break2:
            current_scope = curr_scope;
            break;
    }

    // Cleanup

    if (node->_Node.ForLoop().index_name != nullptr) {
        del_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, current_scope);
    }
    if (node->_Node.ForLoop().value_name != nullptr) {
        del_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, current_scope);
    }

    current_scope = curr_scope;

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_while_loop(node_ptr& node) {
    node_ptr conditional = eval_node(node->_Node.WhileLoop().condition);

    if (conditional->type != NodeType::BOOLEAN) {
        conditional = new_boolean_node(conditional->type != NodeType::NONE);
    }

    auto curr_scope = current_scope;

    while (conditional->_Node.Boolean().value) {
        auto loop_symbol_table = std::make_shared<SymbolTable>();
        for (auto& symbol : current_scope->symbols) {
            loop_symbol_table->symbols[symbol.first] = symbol.second;
        }

        current_scope = loop_symbol_table;
        current_scope->parent = curr_scope;
        
        for (node_ptr expr : node->_Node.WhileLoop().body->_Node.Object().elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                current_scope = curr_scope;
                return evaluated_expr;
            }
            if (evaluated_expr->type == NodeType::BREAK) {
                goto _break;
            }
            if (evaluated_expr->type == NodeType::CONTINUE) {
                break;
            }
        }

        conditional = eval_node(node->_Node.WhileLoop().condition);

        if (conditional->type != NodeType::BOOLEAN) {
            conditional = new_boolean_node(conditional->type != NodeType::NONE);
        }

        current_scope = curr_scope;
    }

    _break:
        current_scope = curr_scope;

    return new_node(NodeType::NONE);
}