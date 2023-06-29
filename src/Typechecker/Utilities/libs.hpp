#pragma once
#include "../Typechecker.hpp"

node_ptr Typechecker::tc_load_lib(node_ptr& node) {
    auto args = node->_Node.FunctionCall().args;
    if (args.size() != 1) {
        return throw_error("Library loading expects 1 argument");
    }

    node_ptr path = tc_node(args[0]);

    if (path->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (path->type != NodeType::STRING) {
        return throw_error("Library loading expects 1 string argument");
    }

    node_ptr lib_node = new_node(NodeType::LIB);

    if (vector_contains_string(node->Meta.tags, "pure")) {
        Interpreter interp;
        lib_node = interp.eval_load_lib(node);
    }

    return lib_node;
}

node_ptr Typechecker::tc_call_lib_function(node_ptr& lib, node_ptr& node) {
    auto args = node->_Node.FunctionCall().args;
    if (args.size() != 2) {
        return throw_error("Library function calls expects 2 arguments");
    }

    node_ptr name = tc_node(args[0]);
    node_ptr func_args = tc_node(args[1]);

    if (name->type == NodeType::ANY || func_args->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (name->type != NodeType::STRING) {
        return throw_error("Library function calls expects first argument to be a string");
    }

    if (func_args->type != NodeType::LIST) {
        return throw_error("Library function calls expects second argument to be a list");
    }

    return new_node(NodeType::ANY);
}