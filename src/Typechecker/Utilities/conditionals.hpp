#pragma once
#include "../Typechecker.hpp"

node_ptr Typechecker::tc_if_statement(node_ptr& node) {

    auto scope = std::make_shared<SymbolTable>();
    scope->parent = current_scope;
    current_scope = scope;

    node_ptr conditional = tc_node(node->_Node.IfStatement().condition);

    std::vector<node_ptr> returns;

   for (node_ptr expr : node->_Node.IfStatement().body->_Node.Object().elements) {
        node_ptr evaluated_expr = tc_node(expr);
        if (evaluated_expr->type == NodeType::RETURN) {
            node_ptr ret_clone = new_node(NodeType::RETURN);
            ret_clone->_Node.Return().value = tc_node(evaluated_expr->_Node.Return().value);
            returns.push_back(ret_clone);
            // current_scope = current_scope->parent;
            // return ret_clone;
        }
        if (evaluated_expr->type == NodeType::PIPE_LIST) {
            node_ptr ret_list = new_node(NodeType::LIST);
            ret_list->type = NodeType::PIPE_LIST;

            for (node_ptr& e : evaluated_expr->_Node.List().elements) {
                if (e->type == NodeType::RETURN) {
                    node_ptr clone = new_node(NodeType::RETURN);
                    clone->_Node.Return().value = tc_node(e->_Node.Return().value);
                    ret_list->_Node.List().elements.push_back(clone);
                }
            }

            ret_list = tc_pipe_list(ret_list);
            returns.push_back(ret_list);
            // current_scope = current_scope->parent;
            // return ret_list;
        }
    }

    for (auto& prop : current_scope->cast_types) {
        *prop.first = *prop.second;
    }

    current_scope = current_scope->parent;
    
    if (returns.size() == 0) {
        return new_node(NodeType::NOVALUE);
    } else if (returns.size() == 1) {
        return returns[0];
    } else {
        node_ptr ret_list = new_node(NodeType::LIST);
        ret_list->type = NodeType::PIPE_LIST;
        ret_list->_Node.List().elements = returns;
        return ret_list;
    }

    return new_node(NodeType::NOVALUE);
}

node_ptr Typechecker::tc_if_block(node_ptr& node) {

    std::vector<node_ptr> results;
    
    for (node_ptr statement : node->_Node.IfBlock().statements) {
        if (statement->type == NodeType::IF_STATEMENT) {
            node_ptr res = tc_node(statement);
            if (res->type == NodeType::PIPE_LIST) {
                for (node_ptr& e : res->_Node.List().elements) {
                    if (e->type == NodeType::RETURN) {
                        results.push_back(e);
                    }
                }
            }
            else if (res->type != NodeType::NOVALUE) {
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
                if (expr->type == NodeType::PIPE_LIST) {
                    for (node_ptr& e : expr->_Node.List().elements) {
                        if (e->type == NodeType::RETURN) {
                            results.push_back(e);
                        }
                    }
                }
                results.push_back(tc_node(expr));
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