#pragma once
#include "../Interpreter.hpp"

node_ptr Interpreter::eval_hook(node_ptr& node) {

    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = node->_Node.Op().right;

    if (left->type == NodeType::_ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::_ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (right->type != NodeType::FUNC_CALL) {
        return throw_error("Right side of hook must be a function call");
    }

    std::string hook_name = right->_Node.FunctionCall().name;

    node_ptr func = nullptr;

    if (right->_Node.FunctionCall().args.size() > 1) {
        return throw_error("Hook call expects 0 or 1 argument(s)");
    }
    
    if (right->_Node.FunctionCall().args.size() == 1) {
        func = eval_node(right->_Node.FunctionCall().args[0]);
    }

    if (func && func->type != NodeType::FUNC) {
        return throw_error("Hook '" + hook_name + "' expects parameter 'func' to be a function");
    }

    if (left->type == NodeType::FUNC) {
        if (hook_name == "onCall") {
            if (!func) {
                // Clear hook
                left->Meta.onCallFunction = nullptr;
                return left;
            }

            if (func->_Node.Function().params.size() != 1) {
                return throw_error("Hook '" + hook_name + "' expects parameter 'func' to have one parameter");
            }

            // Inject function's closure

            for (auto& symbol : current_scope->symbols) {
                func->_Node.Function().closure[symbol.first] = symbol.second;
            }

            // We don't type the function param here, we will do that when it's called

            left->Meta.onCallFunction = func;
            return left;
        }

        return throw_error("No such hook '" + hook_name + "'");
    }

    if (hook_name == "onChange") {
        if (!func) {
            // Clear hook
            left->Meta.onChangeFunction = nullptr;
            return left;
        }

        if (func->_Node.Function().params.size() != 1) {
            return throw_error("Hook '" + hook_name + "' expects parameter 'func' to have one parameter");
        }

        // Inject function's closure

        for (auto& symbol : current_scope->symbols) {
            func->_Node.Function().closure[symbol.first] = symbol.second;
        }

        // We don't type the function param here, we will do that when it's called

        left->Meta.onChangeFunction = func;
        return left;
    }

    if (hook_name == "onInit") {

        if (left->type != NodeType::OBJECT) {
            return throw_error("Cannot apply hook '" + hook_name + "' on non Object type");
        }

        if (!func) {
            // Clear hook
            left->Meta.onInitFunction = nullptr;
            return left;
        }

        if (func->_Node.Function().params.size() != 1) {
            return throw_error("Hook '" + hook_name + "' expects parameter 'func' to have one parameter");
        }

        // Inject function's closure

        for (auto& symbol : current_scope->symbols) {
            func->_Node.Function().closure[symbol.first] = symbol.second;
        }

        // We don't type the function param here, we will do that when it's called

        left->Meta.onInitFunction = func;
        return left;
    }

    return throw_error("No such hook '" + hook_name + "'");
}