#pragma once
#include "../Typechecker.hpp"

node_ptr Typechecker::tc_const_decl(node_ptr& node) {
    std::string var_name = node->_Node.ConstantDeclatation().name;

    node_ptr var = Typechecker::get_symbol_local(var_name, current_scope);

    if (var && var->type != NodeType::FUNC) {
        return Typechecker::throw_error("Variable '" + var_name + "' is already defined");
    }

    node_ptr value = tc_node(node->_Node.ConstantDeclatation().value);

    bool is_ref = value->type == NodeType::REF;

    if (is_ref) {
        value = tc_node(value->_Node.Ref().value);
    }

    if (node->_Node.ConstantDeclatation().type) {
        node->_Node.ConstantDeclatation().type = tc_node(node->_Node.ConstantDeclatation().type);
        node_ptr value_type = get_type(value);
        if (!match_types(node->_Node.ConstantDeclatation().type, value)) {
            return throw_error("Variable '" + var_name + "' expects value of type '" + printable(node->_Node.ConstantDeclatation().type) + "' but received value of type '" + printable(value_type) + "'");
        }
    }

    if (!node->_Node.ConstantDeclatation().type->TypeInfo.is_refinement_type) {
        value->TypeInfo.type = node->_Node.ConstantDeclatation().type;
    }

    if (var && value->type == NodeType::FUNC) {
        var->_Node.Function().dispatch_functions.push_back(value);
        return value;
    }

    value->Meta.is_const = true;

    add_symbol(var_name, value, current_scope);
    return value;
}

node_ptr Typechecker::tc_var_decl(node_ptr& node) {
    std::string var_name = node->_Node.VariableDeclaration().name;

    node_ptr var = Typechecker::get_symbol_local(var_name, current_scope);

    if (var && var->type != NodeType::FUNC) {
        return Typechecker::throw_error("Variable '" + var_name + "' is already defined");
    }

    node_ptr value = tc_node(node->_Node.VariableDeclaration().value);

    bool is_ref = value->type == NodeType::REF;

    if (is_ref) {
        value = tc_node(value->_Node.Ref().value);
    }

    if (node->_Node.VariableDeclaration().type) {
        node->_Node.VariableDeclaration().type = tc_node(node->_Node.VariableDeclaration().type);
        node_ptr value_type = get_type(value);
        if (!match_types(node->_Node.VariableDeclaration().type, value)) {
            return throw_error("Variable '" + var_name + "' expects value of type '" + printable(node->_Node.VariableDeclaration().type) + "' but received value of type '" + printable(value_type) + "'");
        }
    }

    if (!node->_Node.VariableDeclaration().type->TypeInfo.is_refinement_type) {
        value->TypeInfo.type = node->_Node.VariableDeclaration().type;
    }

    if (var && value->type == NodeType::FUNC) {
        var->_Node.Function().dispatch_functions.push_back(value);
        return value;
    }

    add_symbol(var_name, value, current_scope);
    return value;
}