#pragma once
#include "../Typechecker.hpp"

node_ptr Typechecker::tc_for_loop(node_ptr& node) {

    if (node->_Node.ForLoop().iterator != nullptr) {
        node_ptr iterator = tc_node(node->_Node.ForLoop().iterator);

        // TODO: Implement for object looping

        if (iterator->type != NodeType::LIST) {
            return throw_error("For loop iterator must be a list");
        }

        auto curr_scope = current_scope;

        auto loop_symbol_table = std::make_shared<SymbolTable>();
        
        loop_symbol_table->symbols.clear();
        current_scope = loop_symbol_table;
        current_scope->parent = curr_scope;

        int i = 0;

        node_ptr list_type = 
            iterator->TypeInfo.type && iterator->TypeInfo.type->type == NodeType::LIST && iterator->TypeInfo.type->_Node.List().elements.size() == 1 
            ? iterator->TypeInfo.type->_Node.List().elements[0] 
            : new_node(NodeType::ANY);

        if (node->_Node.ForLoop().index_name) {
            node_ptr number_node = new_number_node(i);
            add_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, number_node, current_scope);
        }
        if (node->_Node.ForLoop().value_name) {
            add_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, list_type, current_scope);
        }
        for (node_ptr expr : node->_Node.ForLoop().body->_Node.Object().elements) {
            node_ptr evaluated_expr = tc_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                current_scope = curr_scope;
                return evaluated_expr;
            }
        }

        // Cleanup

        if (node->_Node.ForLoop().index_name != nullptr) {
            del_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, current_scope);
        }
        if (node->_Node.ForLoop().value_name != nullptr) {
            del_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, current_scope);
        }

        current_scope = curr_scope;

        return new_node(NodeType::NOVALUE);
    }

    node_ptr start_node = tc_node(node->_Node.ForLoop().start);
    node_ptr end_node = tc_node(node->_Node.ForLoop().end);

    if (start_node->type != NodeType::NUMBER && end_node->type != NodeType::NUMBER) {
        return throw_error("For loop range expects two numbers");
    }

    int start = start_node->_Node.Number().value;
    int end = end_node->_Node.Number().value;

    auto curr_scope = current_scope;

    auto loop_symbol_table = std::make_shared<SymbolTable>();
    
    loop_symbol_table->symbols.clear();
    current_scope = loop_symbol_table;
    current_scope->parent = curr_scope;

    loop_symbol_table->symbols.clear();

    int index = 0;

    if (node->_Node.ForLoop().index_name) {
        node_ptr index_node = new_number_node(index);
        current_scope->symbols[node->_Node.ForLoop().index_name->_Node.ID().value] = index_node;
    }
    if (node->_Node.ForLoop().value_name) {
        node_ptr value_node = new_number_node(index);
        current_scope->symbols[node->_Node.ForLoop().value_name->_Node.ID().value] = value_node;
    }
    for (node_ptr& expr : node->_Node.ForLoop().body->_Node.Object().elements) {
        node_ptr evaluated_expr = tc_node(expr);
        if (evaluated_expr->type == NodeType::RETURN) {
            current_scope = curr_scope;
            return evaluated_expr;
        }
    }

    // Cleanup

    if (node->_Node.ForLoop().index_name != nullptr) {
        del_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, current_scope);
    }
    if (node->_Node.ForLoop().value_name != nullptr) {
        del_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, current_scope);
    }

    current_scope = curr_scope;

    return new_node(NodeType::NOVALUE);
}

node_ptr Typechecker::tc_while_loop(node_ptr& node) {

    node_ptr conditional = tc_node(node->_Node.WhileLoop().condition);

    auto curr_scope = current_scope;

    auto loop_symbol_table = std::make_shared<SymbolTable>();
    for (auto& symbol : current_scope->symbols) {
        loop_symbol_table->symbols[symbol.first] = symbol.second;
    }

    current_scope = loop_symbol_table;
    current_scope->parent = curr_scope;
    
    for (node_ptr expr : node->_Node.WhileLoop().body->_Node.Object().elements) {
        node_ptr evaluated_expr = tc_node(expr);
        if (evaluated_expr->type == NodeType::RETURN) {
            current_scope = curr_scope;
            return evaluated_expr;
        }
    }

    current_scope = curr_scope;

    return new_node(NodeType::NOVALUE);
}