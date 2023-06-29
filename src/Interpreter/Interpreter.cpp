#include "Interpreter.hpp"
#include "Utilities/operations.hpp"
#include "Utilities/equals.hpp"
#include "Utilities/decl.hpp"
#include "Utilities/types.hpp"
#include "Utilities/objects.hpp"
#include "Utilities/functions.hpp"
#include "Utilities/conditionals.hpp"
#include "Utilities/loops.hpp"
#include "Utilities/errors.hpp"
#include "Utilities/imports.hpp"
#include "Utilities/dot.hpp"
#include "Utilities/libs.hpp"
#include "Utilities/hooks.hpp"
#include "Utilities/print.hpp"

void Interpreter::advance(int n) {
    index += n;
    if (index < nodes.size()) {
        current_node = nodes[index];
        line = current_node->line;
        column = current_node->column;
    }
};

node_ptr Interpreter::peek(int n) {
    int idx = index + n;
    if (idx < nodes.size()) {
        return nodes[idx];
    }
    return nullptr;
}

void Interpreter::reset(int idx) {
    index = idx;
    current_node = nodes[index];
}

void Interpreter::add_symbol(std::string name, node_ptr value, symt_ptr scope) {
    scope->symbols[name] = value;
}

void Interpreter::del_symbol(std::string name, symt_ptr scope) {
    scope->symbols.erase(name);
}

node_ptr Interpreter::get_symbol_local(std::string name, symt_ptr scope) {
    return scope->symbols[name];
}

node_ptr Interpreter::get_symbol(std::string name, symt_ptr scope) {
    symt_ptr current_symtable = scope;
    while (current_symtable) {
        if (current_symtable->symbols.count(name)) {
            return current_symtable->symbols[name];
        }

        current_symtable = current_symtable->parent;
    }

    return nullptr;
}

void Interpreter::error_and_exit(std::string message, node_ptr node)
{
    if (node) {
        line = node->line;
        column = node->column;
    }
    
    symt_ptr curr_scope = current_scope;
    while (curr_scope) {
        std::string error_message = "\nError in '" + curr_scope->file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	    std::cout << error_message << "\n";
        curr_scope = curr_scope->parent;
    }
    exit(1);
}

node_ptr Interpreter::throw_error(std::string message, node_ptr node)
{
    if (node) {
        line = node->line;
        column = node->column;
    }
    
    std::string error_message = "\nError in '" + current_scope->file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;

    if (global_interpreter->try_catch > 0) {
        node_ptr error = new_node(NodeType::ERROR);
        error->_Node.Error().message = error_message;
        global_interpreter->error = error_message;
        return error;
    } else {
        error_and_exit(message);
        return new_node(NodeType::ERROR);
    }
}

// Helpers

node_ptr Interpreter::new_number_node(double value) {
    auto node = std::make_shared<Node>(NodeType::NUMBER);
    node->Meta.is_const = false;
    node->_Node.Number().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_string_node(std::string value) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->Meta.is_const = false;
    node->_Node.String().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_boolean_node(bool value) {
    auto node = std::make_shared<Node>(NodeType::BOOLEAN);
    node->Meta.is_const = false;
    node->_Node.Boolean().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_accessor_node() {
    auto node = std::make_shared<Node>(NodeType::ACCESSOR);
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_list_node(std::vector<node_ptr> nodes) {
    auto node = std::make_shared<Node>(NodeType::LIST);
    node->_Node.List().elements = nodes;
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_node(NodeType type) {
    auto node = std::make_shared<Node>(type);
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_node() {
    auto node = std::make_shared<Node>();
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::eval_node(node_ptr& node) {

    if (global_interpreter->try_catch > 0 && global_interpreter->error != "") {
        return throw_error(global_interpreter->error);
    }

    if (node == nullptr) {
        return nullptr;
    }

    line = node->line;
    column = node->column;

    if (node->Meta.evaluated) {
        return node;
    }

    switch (node->type) {
        case NodeType::NUMBER: {
            return node;
        }
        case NodeType::STRING: {
            return node;
        }
        case NodeType::BOOLEAN: {
            return node;
        }
        case NodeType::LIST: {
            return eval_list(node);
        }
        case NodeType::PIPE_LIST: {
            return eval_pipe_list(node);
        }
        case NodeType::OBJECT: {
            if (node->_Node.Object().elements.size() == 1 && (node->_Node.Object().elements[0]->type == NodeType::COMMA_LIST || node->_Node.Object().elements[0]->_Node.Op().value == ":")) {
                return eval_object(node);
            }
            return node;
        }
        case NodeType::ID: {
            node_ptr value = get_symbol(node->_Node.ID().value, current_scope);
            if (value == nullptr) {
                return throw_error("Variable '" + node->_Node.ID().value + "' is undefined");
            }
            return value;
        }
        case NodeType::PAREN: {
            if (node->_Node.Paren().elements.size() != 1) {
                return new_node(NodeType::NONE);
            }
            return eval_node(node->_Node.Paren().elements[0]);
        }
        case NodeType::FUNC: {
            return eval_function(node);
        }
        case NodeType::CONSTANT_DECLARATION: {
            return eval_const_decl(node);
        }
        case NodeType::VARIABLE_DECLARATION: {
            return eval_var_decl(node);
        }
        case NodeType::FUNC_CALL: {
            return eval_func_call(node);
        }
        case NodeType::IF_STATEMENT: {
            return eval_if_statement(node);
        }
        case NodeType::IF_BLOCK: {
            return eval_if_block(node);
        }
        case NodeType::WHILE_LOOP: {
            return eval_while_loop(node);
        }
        case NodeType::FOR_LOOP: {
            return eval_for_loop(node);
        }
        case NodeType::ACCESSOR: {
            return eval_accessor(node);
        }
        case NodeType::IMPORT: {
            return eval_import(node);
        }
        case NodeType::TYPE: {
            return eval_type(node);
        }
        case NodeType::OBJECT_DECONSTRUCT: {
            return eval_object_init(node);
        }
        case NodeType::TRY_CATCH: {
            return eval_try_catch(node);
        }
        case NodeType::OP: {
            if ((node->_Node.Op().value == "+" || node->_Node.Op().value == "-") 
                && node->_Node.Op().left == nullptr) {
                return eval_pos_neg(node);
            }
            if (node->_Node.Op().value == "=") {
                return eval_eq(node);
            }
            if (node->_Node.Op().value == ".") {
                return eval_dot(node);
            }
            if (node->_Node.Op().value == "::") {
                return eval_hook(node);
            }
            if (node->_Node.Op().value == "+") {
                return eval_add(node);
            }
            if (node->_Node.Op().value == "-") {
                return eval_sub(node);
            }
            if (node->_Node.Op().value == "*") {
                return eval_mul(node);
            }
            if (node->_Node.Op().value == "/") {
                return eval_div(node);
            }
            if (node->_Node.Op().value == "^") {
                return eval_pow(node);
            }
            if (node->_Node.Op().value == "%") {
                return eval_mod(node);
            }
            if (node->_Node.Op().value == "!") {
                return eval_not(node);
            }
            if (node->_Node.Op().value == "==") {
                return eval_eq_eq(node);
            }
            if (node->_Node.Op().value == "!=") {
                return eval_not_eq(node);
            }
            if (node->_Node.Op().value == "<=") {
                return eval_lt_eq(node);
            }
            if (node->_Node.Op().value == ">=") {
                return eval_gt_eq(node);
            }
            if (node->_Node.Op().value == "<") {
                return eval_lt(node);
            }
            if (node->_Node.Op().value == ">") {
                return eval_gt(node);
            }
            if (node->_Node.Op().value == "&&") {
                return eval_and(node);
            }
            if (node->_Node.Op().value == "||") {
                return eval_or(node);
            }
            if (node->_Node.Op().value == "??") {
                return eval_null_op(node);
            }
            if (node->_Node.Op().value == "and") {
                return eval_bit_and(node);
            }
            if (node->_Node.Op().value == "or") {
                return eval_bit_or(node);
            }
            if (node->_Node.Op().value == "+=") {
                return eval_plus_eq(node);
            }
            if (node->_Node.Op().value == "-=") {
                return eval_minus_eq(node);
            }
            if (node->_Node.Op().value == "as") {
                return eval_as(node);
            }
            if (node->_Node.Op().value == "is") {
                return eval_is(node);
            }
            if (node->_Node.Op().value == "in") {
                return eval_in(node);
            }
            if (node->_Node.Op().value == ";") {
                return node;
            }
            
            return throw_error("No such operator '" + node->_Node.Op().value + "'");
        }
        default: {
            return node;
        }
    }

    return node;
}

void Interpreter::evaluate() {
    while (current_node->type != NodeType::END_OF_FILE) {
        node_ptr evaluated_node = eval_node(current_node);
        if (evaluated_node->type == NodeType::ERROR) {
            error_and_exit(evaluated_node->_Node.Error().message);
        }
        advance();
    }
}

 std::vector<node_ptr> Interpreter::sort_and_unique(std::vector<node_ptr>& list) {

    for (node_ptr& elem : list) {
        if (elem->type == NodeType::ANY) {
            std::vector<node_ptr> ls = {new_node(NodeType::ANY)};
            return ls;
        }
    }

    sort(list.begin(), list.end(), 
        [this](node_ptr& a, node_ptr& b)
        { 
            return a->type < b->type;
        });

    std::vector<node_ptr> unique_list;

    for (node_ptr& elem : list) {
        bool match = false;
        if (unique_list.size() == 0) {
            unique_list.push_back(elem);
            continue;
        } else {
            for (node_ptr& e: unique_list) {
                if (match_types(e, elem)) {
                    match = true;
                    continue;
                }
            }
        }

        if (!match) {
            unique_list.push_back(elem);
        }
    }

    return unique_list;

    // return list;
 }