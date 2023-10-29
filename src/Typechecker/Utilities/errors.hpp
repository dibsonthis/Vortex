#pragma once
#include "../Typechecker.hpp"

node_ptr Typechecker::tc_try_catch(node_ptr& node) {
    std::vector<node_ptr> returns;
    std::shared_ptr<SymbolTable> try_sym_table = std::make_shared<SymbolTable>();
    try_sym_table->parent = current_scope;
    current_scope = try_sym_table;

    std::string error = "";
    for (node_ptr& expr : node->_Node.TryCatch().try_body->_Node.Object().elements) {
        node_ptr evaluated_expr = tc_node(expr);
        if (evaluated_expr->type == NodeType::_ERROR) {
            error = evaluated_expr->_Node.Error().message;
            return evaluated_expr;
        }
        if (evaluated_expr->type == NodeType::RETURN) {
            evaluated_expr->_Node.Return().value = tc_node(evaluated_expr->_Node.Return().value);
            returns.push_back(evaluated_expr);
        }
    }

    current_scope = current_scope->parent;

    std::shared_ptr<SymbolTable> catch_sym_table = std::make_shared<SymbolTable>();
    catch_sym_table->parent = current_scope;
    node_ptr error_arg = node->_Node.TryCatch().catch_keyword->_Node.FunctionCall().args[0];
    node_ptr error_node = new_string_node(error);
    add_symbol(error_arg->_Node.ID().value, error_node, catch_sym_table);
    current_scope = catch_sym_table;

    for (node_ptr& expr : node->_Node.TryCatch().catch_body->_Node.Object().elements) {
        node_ptr evaluated_expr = tc_node(expr);
        if (evaluated_expr->type == NodeType::RETURN) {
            evaluated_expr->_Node.Return().value = tc_node(evaluated_expr->_Node.Return().value);
            returns.push_back(evaluated_expr);
        }
    }

    current_scope = current_scope->parent;

    if (returns.size() == 0) {
        return new_node(NodeType::NOVALUE);
    }
    
    node_ptr union_list = new_node(NodeType::LIST);
    union_list->type = NodeType::PIPE_LIST;
    union_list->_Node.List().elements = returns;
    union_list->TypeInfo.is_type = true;

    return union_list;
}