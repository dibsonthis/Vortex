#include "Typechecker.hpp"
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

void Typechecker::advance(int n) {
    index += n;
    if (index < nodes.size()) {
        current_node = nodes[index];
        line = current_node->line;
        column = current_node->column;
    }
};

node_ptr Typechecker::peek(int n) {
    int idx = index + n;
    if (idx < nodes.size()) {
        return nodes[idx];
    }
    return nullptr;
}

void Typechecker::reset(int idx) {
    index = idx;
    current_node = nodes[index];
}

void Typechecker::add_symbol(std::string name, node_ptr value, symt_ptr scope) {
    scope->symbols[name] = value;
}

void Typechecker::del_symbol(std::string name, symt_ptr scope) {
    scope->symbols.erase(name);
}

node_ptr Typechecker::get_symbol_local(std::string name, symt_ptr scope) {
    return scope->symbols[name];
}

node_ptr Typechecker::get_symbol(std::string name, symt_ptr scope) {
    symt_ptr current_symtable = scope;
    while (current_symtable) {
        if (current_symtable->symbols.count(name)) {
            return current_symtable->symbols[name];
        }

        current_symtable = current_symtable->parent;
    }

    return nullptr;
}

void Typechecker::error_and_exit(std::string message, node_ptr node)
{
    if (node) {
        line = node->line;
        column = node->column;
    }

    symt_ptr curr_scope = current_scope;
    while (curr_scope) {
        std::string error_message = "\nError in '" + curr_scope->file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	    std::cout << error_message;
        curr_scope = curr_scope->parent;
    }
    exit(1);
}

node_ptr Typechecker::throw_error(std::string message, node_ptr node)
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

node_ptr Typechecker::new_number_node(double value) {
    auto node = std::make_shared<Node>(NodeType::NUMBER);
    node->Meta.is_const = false;
    node->_Node.Number().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Typechecker::new_string_node(std::string value) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->Meta.is_const = false;
    node->_Node.String().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Typechecker::new_boolean_node(bool value) {
    auto node = std::make_shared<Node>(NodeType::BOOLEAN);
    node->Meta.is_const = false;
    node->_Node.Boolean().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Typechecker::new_accessor_node() {
    auto node = std::make_shared<Node>(NodeType::ACCESSOR);
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Typechecker::new_list_node(std::vector<node_ptr> nodes) {
    auto node = std::make_shared<Node>(NodeType::LIST);
    node->_Node.List().elements = nodes;
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Typechecker::new_node(NodeType type) {
    auto node = std::make_shared<Node>(type);
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Typechecker::new_node() {
    auto node = std::make_shared<Node>();
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Typechecker::tc_node(node_ptr& node) {

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
            return tc_list(node);
        }
        case NodeType::PIPE_LIST: {
            return tc_pipe_list(node);
        }
        case NodeType::OBJECT: {
            if (node->_Node.Object().elements.size() == 1 && (node->_Node.Object().elements[0]->type == NodeType::COMMA_LIST || node->_Node.Object().elements[0]->type == NodeType::OP && node->_Node.Object().elements[0]->_Node.Op().value == ":")) {
                return tc_object(node);
            }  else if (node->_Node.Object().elements.size() == 1) {
                node_ptr obj = new_node(NodeType::OBJECT);
                obj->Meta = node->Meta;
                obj->_Node.Object().elements.push_back(tc_node(node->_Node.Object().elements[0]));
                return obj;
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
            return tc_node(node->_Node.Paren().elements[0]);
        }
        case NodeType::FUNC: {
            return tc_function(node);
        }
        case NodeType::CONSTANT_DECLARATION: {
            return tc_const_decl(node);
        }
        case NodeType::VARIABLE_DECLARATION: {
            return tc_var_decl(node);
        }
        case NodeType::FUNC_CALL: {
            return tc_func_call(node);
        }
        case NodeType::IF_STATEMENT: {
            return tc_if_statement(node);
        }
        case NodeType::IF_BLOCK: {
            return tc_if_block(node);
        }
        case NodeType::WHILE_LOOP: {
            return tc_while_loop(node);
        }
        case NodeType::FOR_LOOP: {
            return tc_for_loop(node);
        }
        case NodeType::ACCESSOR: {
            return tc_accessor(node);
        }
        case NodeType::IMPORT: {
            return tc_import(node);
        }
        case NodeType::TYPE: {
            return tc_type(node);
        }
        case NodeType::OBJECT_DECONSTRUCT: {
            return tc_object_init(node);
        }
        case NodeType::TRY_CATCH: {
            return tc_try_catch(node);
        }
        case NodeType::OP: {
            if ((node->_Node.Op().value == "+" || node->_Node.Op().value == "-") 
                && node->_Node.Op().left == nullptr) {
                return tc_pos_neg(node);
            }
            if (node->_Node.Op().value == "=") {
                return tc_eq(node);
            }
            if (node->_Node.Op().value == ".") {
                return tc_dot(node);
            }
            if (node->_Node.Op().value == "::") {
                return tc_hook(node);
            }
            if (node->_Node.Op().value == "+") {
                return tc_add(node);
            }
            if (node->_Node.Op().value == "-") {
                return tc_sub(node);
            }
            if (node->_Node.Op().value == "*") {
                return tc_mul(node);
            }
            if (node->_Node.Op().value == "/") {
                return tc_div(node);
            }
            if (node->_Node.Op().value == "^") {
                return tc_pow(node);
            }
            if (node->_Node.Op().value == "%") {
                return tc_mod(node);
            }
            if (node->_Node.Op().value == "!") {
                return tc_not(node);
            }
            if (node->_Node.Op().value == "==") {
                return tc_eq_eq(node);
            }
            if (node->_Node.Op().value == "!=") {
                return tc_not_eq(node);
            }
            if (node->_Node.Op().value == "<=") {
                return tc_lt_eq(node);
            }
            if (node->_Node.Op().value == ">=") {
                return tc_gt_eq(node);
            }
            if (node->_Node.Op().value == "<") {
                return tc_lt(node);
            }
            if (node->_Node.Op().value == ">") {
                return tc_gt(node);
            }
            if (node->_Node.Op().value == "&&") {
                return tc_and(node);
            }
            if (node->_Node.Op().value == "||") {
                return tc_or(node);
            }
            if (node->_Node.Op().value == "??") {
                return tc_null_op(node);
            }
            if (node->_Node.Op().value == "and") {
                return tc_bit_and(node);
            }
            if (node->_Node.Op().value == "or") {
                return tc_bit_or(node);
            }
            if (node->_Node.Op().value == "+=") {
                return tc_plus_eq(node);
            }
            if (node->_Node.Op().value == "-=") {
                return tc_minus_eq(node);
            }
            if (node->_Node.Op().value == "as") {
                return tc_as(node);
            }
            if (node->_Node.Op().value == "is") {
                return tc_is(node);
            }
            if (node->_Node.Op().value == "in") {
                return tc_in(node);
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

void Typechecker::typecheck() {
    while (current_node->type != NodeType::END_OF_FILE) {
        node_ptr evaluated_node = tc_node(current_node);
        if (evaluated_node->type == NodeType::ERROR) {
            error_and_exit(evaluated_node->_Node.Error().message);
        }
        advance();
    }
}

 std::vector<node_ptr> Typechecker::sort_and_unique(std::vector<node_ptr>& list) {

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

    for (node_ptr elem : list) {
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
 }