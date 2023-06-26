#pragma once
#include "../Typechecker.hpp"

node_ptr Typechecker::tc_if_statement(node_ptr& node) {

    auto scope = std::make_shared<SymbolTable>();
    scope->parent = current_scope;
    current_scope = scope;

    node_ptr conditional = tc_node(node->_Node.IfStatement().condition);

   for (node_ptr expr : node->_Node.IfStatement().body->_Node.Object().elements) {
        node_ptr evaluated_expr = tc_node(expr);
        if (evaluated_expr->type == NodeType::RETURN) {
            evaluated_expr->_Node.Return().value = tc_node(evaluated_expr->_Node.Return().value);
            current_scope = current_scope->parent;
            return evaluated_expr;
        }
    }

    for (auto& prop : current_scope->cast_types) {
        *prop.first = *prop.second;
    }

    current_scope = current_scope->parent;

    return new_node(NodeType::NOVALUE);
}

node_ptr Typechecker::tc_if_block(node_ptr& node) {

    std::vector<node_ptr> results;
    
    for (node_ptr statement : node->_Node.IfBlock().statements) {
        if (statement->type == NodeType::IF_STATEMENT) {
            //node_ptr conditional = tc_node(statement->_Node.IfStatement().condition);
            node_ptr res = tc_node(statement);
            if (res->type != NodeType::NOVALUE) {
                results.push_back(res);
            }
        } else if (statement->type == NodeType::OBJECT) {

            auto scope = std::make_shared<SymbolTable>();
            scope->parent = current_scope;
            current_scope = scope;

            for (node_ptr expr : statement->_Node.Object().elements) {
                if (expr->type == NodeType::RETURN) {
                    node_ptr res = tc_node(expr);
                    if (res->type != NodeType::NOVALUE) {
                        results.push_back(res);
                    }
                }
                tc_node(expr);
            }

            current_scope = current_scope->parent;

            if (results.size() > 0) {
                node_ptr res_union = new_node(NodeType::LIST);
                res_union->type = NodeType::PIPE_LIST;
                res_union->_Node.List().elements = results;
                return res_union;
            }
            return new_node(NodeType::NOVALUE);
        }
    }

    if (results.size() > 0) {
        node_ptr res_union = new_node(NodeType::LIST);
        res_union->type = NodeType::PIPE_LIST;
        res_union->_Node.List().elements = results;
        return res_union;
    }

    return new_node(NodeType::NOVALUE);
}