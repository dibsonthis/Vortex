#pragma once

#define GCC_COMPILER (defined(__GNUC__) && !defined(__clang__))

#if GCC_COMPILER
    #if __apple__ || __linux__
        #include <dlfcn.h>
    #else
        #include <windows.h>
    #endif
#else
    #if defined(__APPLE__) || defined(__linux__)
        #include <dlfcn.h>
    #else
        #include <windows.h>
    #endif
#endif

#include <memory>
#include <unordered_map>
#include <sstream>
#include <cmath>
#include <thread>
#include <future>
#include "../Node/Node.hpp"

#define symt_ptr std::shared_ptr<SymbolTable>

struct SymbolTable {
    symt_ptr parent;
    symt_ptr child;
    std::unordered_map<std::string, node_ptr> symbols;
    std::string file_name;
    std::unordered_map<node_ptr, node_ptr> cast_types;
};

struct Interpreter {

public:
    symt_ptr global_scope = std::make_shared<SymbolTable>();;
    symt_ptr current_scope = global_scope;

    Interpreter* global_interpreter = this;

    std::vector<node_ptr> nodes;
    node_ptr current_node;
    int index = 0;
    std::string file_name;
    int line, column;
    int argc;
    std::vector<std::string> argv;
    int try_catch = 0;
    std::string error = "";

    // Threading API
    std::unordered_map<uint32_t, std::shared_future<node_ptr>> _futures;

    Interpreter() = default;

    Interpreter(std::vector<node_ptr> nodes) : nodes(nodes) {
        current_node = nodes[0];
        current_scope->file_name = file_name;
    }

    Interpreter(std::vector<node_ptr> nodes, std::string file_name) : nodes(nodes), file_name(file_name) {
        current_node = nodes[0];
        current_scope->file_name = file_name;
    }

    void advance(int n = 1);
    node_ptr peek(int n = 1);
    void reset(int idx = 0);

    void add_symbol(std::string name, node_ptr value, symt_ptr scope);
    void del_symbol(std::string name, symt_ptr scope);
    node_ptr get_symbol_local(std::string name, symt_ptr scope);
    node_ptr get_symbol(std::string name, symt_ptr scope);

    void evaluate();
    node_ptr eval_node(node_ptr& node);

    void error_and_exit(std::string message, node_ptr node = nullptr);
    node_ptr throw_error(std::string message, node_ptr node = nullptr);

    // Helpers

    node_ptr new_number_node(double value);
    node_ptr new_string_node(std::string value);
    node_ptr new_boolean_node(bool value);
    node_ptr new_accessor_node();
    node_ptr new_list_node(std::vector<node_ptr> nodes);
    node_ptr new_node(NodeType type);
    node_ptr new_node();

    // Operations

    node_ptr eval_pos_neg(node_ptr& node);
    node_ptr eval_not(node_ptr& node);
    node_ptr eval_add(node_ptr& node);
    node_ptr eval_sub(node_ptr& node);
    node_ptr eval_mul(node_ptr& node);
    node_ptr eval_div(node_ptr& node);
    node_ptr eval_pow(node_ptr& node);
    node_ptr eval_mod(node_ptr& node);
    node_ptr eval_eq_eq(node_ptr& node);
    node_ptr eval_not_eq(node_ptr& node);
    node_ptr eval_lt_eq(node_ptr& node);
    node_ptr eval_gt_eq(node_ptr& node);
    node_ptr eval_lt(node_ptr& node);
    node_ptr eval_gt(node_ptr& node);
    node_ptr eval_and(node_ptr& node);
    node_ptr eval_or(node_ptr& node);
    node_ptr eval_null_op(node_ptr& node);
    node_ptr eval_plus_eq(node_ptr& node);
    node_ptr eval_minus_eq(node_ptr& node);
    node_ptr eval_bit_and(node_ptr& node);
    node_ptr eval_bit_or(node_ptr& node);
    node_ptr eval_as(node_ptr& node);
    node_ptr eval_is(node_ptr& node);
    node_ptr eval_in(node_ptr& node);

    node_ptr eval_eq(node_ptr& node);
    node_ptr eval_eq_id(node_ptr& id, node_ptr& value, bool is_ref);
    node_ptr eval_eq_dot(node_ptr& left, node_ptr& value, bool is_ref);
    node_ptr eval_eq_accessor(node_ptr& left, node_ptr& value, bool is_ref);

    node_ptr eval_const_decl(node_ptr& node);
    node_ptr eval_var_decl(node_ptr& node);

    node_ptr eval_object(node_ptr& node);
    node_ptr eval_list(node_ptr& node);
    node_ptr eval_pipe_list(node_ptr& node);
    node_ptr eval_accessor(node_ptr& node);
    node_ptr eval_type(node_ptr& node);
    node_ptr eval_object_init(node_ptr& node);

    node_ptr eval_function(node_ptr& node);
    node_ptr eval_func_call(node_ptr& node, node_ptr func);
    node_ptr eval_function_direct(node_ptr& func_call, node_ptr& function);

    node_ptr eval_if_statement(node_ptr& node);
    node_ptr eval_if_block(node_ptr& node);

    node_ptr eval_for_loop(node_ptr& node);
    node_ptr eval_while_loop(node_ptr& node);

    node_ptr eval_try_catch(node_ptr& node);
    node_ptr eval_import(node_ptr& node);

    node_ptr eval_dot(node_ptr& node);
    node_ptr eval_dot_string(node_ptr& left, node_ptr& right);
    node_ptr eval_dot_list(node_ptr& left, node_ptr& right);
    node_ptr eval_dot_object(node_ptr& left, node_ptr& right);
    node_ptr eval_dot_function(node_ptr& left, node_ptr& right);
    // List methods
    node_ptr list_method_append(node_ptr& list, node_ptr& list_type, node_ptr& value);
    node_ptr list_method_prepend(node_ptr& list, node_ptr& list_type, node_ptr& value);
    node_ptr list_method_insert(node_ptr& list, node_ptr& list_type, node_ptr& value, int index);
    node_ptr list_method_update(node_ptr& list, node_ptr& list_type, node_ptr& value, int index);
    node_ptr list_method_remove_at(node_ptr& list, int index);
    node_ptr list_method_remove(node_ptr& list, node_ptr& value);
    // Functional operations
    node_ptr list_method_foreach(node_ptr& list, node_ptr& function);
    node_ptr list_method_map(node_ptr& list, node_ptr& function);
    node_ptr list_method_filter(node_ptr& list, node_ptr& function);
    node_ptr list_method_reduce(node_ptr& list, node_ptr& function);

    node_ptr eval_load_lib(node_ptr& node);
    node_ptr eval_call_lib_function(node_ptr& lib, node_ptr& node);

    node_ptr eval_hook(node_ptr& node);

    node_ptr get_type(node_ptr& node, std::vector<node_ptr> bases);
    bool match_types(node_ptr& _type, node_ptr& _value);
    bool match_values(node_ptr& nodeA, node_ptr& nodeB);
    bool match_inferred_types(node_ptr& nodeA, node_ptr& nodeB);
    bool compareNodeTypes(node_ptr& lhs, node_ptr& rhs);

    node_ptr copy_function(node_ptr& func);

    std::string printable(node_ptr& node, std::vector<node_ptr> bases = {});

    std::vector<node_ptr> sort_and_unique(std::vector<node_ptr>& list);
};