#pragma once
#include "../Interpreter.hpp"

node_ptr Interpreter::eval_const_decl(node_ptr& node) {
    std::string var_name = node->_Node.ConstantDeclatation().name;

    node_ptr var = Interpreter::get_symbol_local(var_name, current_scope);

    if (var && var->type != NodeType::FUNC) {
        return Interpreter::throw_error("Variable '" + var_name + "' is already defined");
    }

    node_ptr copy = std::make_shared<Node>(*node->_Node.ConstantDeclatation().value);
    node_ptr value = eval_node(copy);

    bool is_ref = value->type == NodeType::REF;

    if (is_ref) {
        value = eval_node(value->_Node.Ref().value);
    }

    if (node->_Node.ConstantDeclatation().type) {
        node->_Node.ConstantDeclatation().type = eval_node(node->_Node.ConstantDeclatation().type);
        node_ptr value_type = get_type(value);
        if (!match_types(node->_Node.ConstantDeclatation().type, value_type)) {
            return throw_error("Variable '" + var_name + "' expects value of type '" + printable(node->_Node.ConstantDeclatation().type) + "' but received value of type '" + printable(value_type) + "'");
        }
    }

    if (!node->_Node.ConstantDeclatation().type->TypeInfo.is_refinement_type) {
        value->TypeInfo.type = node->_Node.ConstantDeclatation().type;
    } else {
        std::string first_param_name = node->_Node.ConstantDeclatation().type->_Node.Function().params[0]->_Node.ID().value;
        value->TypeInfo.type = node->_Node.ConstantDeclatation().type->_Node.Function().param_types[first_param_name];
    }

    if (var && value->type == NodeType::FUNC) {
        var->_Node.Function().dispatch_functions.push_back(value);
        return value;
    }

    value->Meta.is_const = true;

    add_symbol(var_name, value, current_scope);
    return value;
}

node_ptr Interpreter::eval_var_decl(node_ptr& node) {
    std::string var_name = node->_Node.VariableDeclaration().name;

    node_ptr var = Interpreter::get_symbol_local(var_name, current_scope);

    if (var && var->type != NodeType::FUNC) {
        return Interpreter::throw_error("Variable '" + var_name + "' is already defined");
    }
    node_ptr copy = std::make_shared<Node>(*node->_Node.VariableDeclaration().value);
    node_ptr value = eval_node(copy);

    bool is_ref = value->type == NodeType::REF;

    if (is_ref) {
        value = eval_node(value->_Node.Ref().value);
    }

    if (node->_Node.VariableDeclaration().type) {
        node->_Node.VariableDeclaration().type = eval_node(node->_Node.VariableDeclaration().type);
        node_ptr value_type = get_type(value);
        if (!match_types(node->_Node.VariableDeclaration().type, value_type)) {
            return throw_error("Variable '" + var_name + "' expects value of type '" + printable(node->_Node.VariableDeclaration().type) + "' but received value of type '" + printable(value_type) + "'");
        }
    }

    if (!node->_Node.ConstantDeclatation().type->TypeInfo.is_refinement_type) {
        value->TypeInfo.type = node->_Node.ConstantDeclatation().type;
    } else {
        std::string first_param_name = node->_Node.ConstantDeclatation().type->_Node.Function().params[0]->_Node.ID().value;
        value->TypeInfo.type = node->_Node.ConstantDeclatation().type->_Node.Function().param_types[first_param_name];
    }

    if (var && value->type == NodeType::FUNC) {
        var->_Node.Function().dispatch_functions.push_back(value);
        return value;
    }

    add_symbol(var_name, value, current_scope);
    return value;
}