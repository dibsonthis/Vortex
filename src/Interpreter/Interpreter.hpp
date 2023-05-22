#pragma once

#define GCC_COMPILER (defined(__GNUC__) && !defined(__clang__))

#define sym_t_ptr std::shared_ptr<SymbolTable>

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

#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <math.h>
#include <thread>
#include <future>
#include "../Node/Node.hpp"
#include "../Lexer/Lexer.hpp"
#include "../Parser/Parser.hpp"
#include "../utils/utils.hpp"
struct Symbol {
    std::string name;
    node_ptr value;
    node_ptr type;
};

struct SymbolTable {
    std::unordered_map<std::string, Symbol> symbols;
    sym_t_ptr parent = nullptr;
    sym_t_ptr child = nullptr;

    // Hooks
    std::vector<node_ptr> globalHooks_onChange;
    std::vector<node_ptr> globalHooks_onCall;

    // Scope info
    std::string filename;

    // Base type extensions
    std::unordered_map<std::string, node_ptr> StringExtensions;
    std::unordered_map<std::string, node_ptr> NumberExtensions;
    std::unordered_map<std::string, node_ptr> ListExtensions;
    std::unordered_map<std::string, node_ptr> BoolExtensions;
    std::unordered_map<std::string, node_ptr> ObjectExtensions;
    std::unordered_map<std::string, node_ptr> FunctionExtensions;
    std::unordered_map<std::string, node_ptr> NoneExtensions;
    std::unordered_map<std::string, std::unordered_map<std::string, node_ptr>> CustomTypeExtensions;
};

class Interpreter {
public:
    std::vector<node_ptr> nodes;
    node_ptr current_node;
    int index = 0;
    std::string file_name;
    int line, column;
    sym_t_ptr global_symbol_table = std::make_shared<SymbolTable>();
    sym_t_ptr current_symbol_table = global_symbol_table;
    Interpreter* global_interpreter = this;
    int argc;
    std::vector<std::string> argv;
    bool try_catch = false;
    // Futures
    std::unordered_map<uint32_t, std::shared_future<node_ptr>> _futures;

public:
    Interpreter() = default;
    Interpreter(std::vector<node_ptr> nodes) : nodes(nodes) {
        current_node = nodes[0];
    }
    Interpreter(std::vector<node_ptr> nodes, std::string file_name) : nodes(nodes), file_name(file_name) {
        current_node = nodes[0];
    }
    void advance(int n = 1);
    node_ptr peek(int n = 1);
    void reset(int idx = 0);

    void evaluate();
    node_ptr eval_node(node_ptr& node);
    node_ptr eval_const_decl(node_ptr& node);
    node_ptr eval_var_decl(node_ptr& node);
    node_ptr eval_list(node_ptr& node);
    node_ptr eval_object(node_ptr& node);
    node_ptr eval_function(node_ptr& node);
    node_ptr eval_func_call(node_ptr& node, node_ptr func = nullptr);
    node_ptr eval_if_statement(node_ptr& node);
    node_ptr eval_if_block(node_ptr& node);
    node_ptr eval_return(node_ptr& node);
    node_ptr eval_for_loop(node_ptr& node);
    node_ptr eval_while_loop(node_ptr& node);
    node_ptr eval_accessor(node_ptr& node);
    node_ptr eval_import(node_ptr& node);
    node_ptr eval_type(node_ptr& node);
    node_ptr eval_type_ext(node_ptr& node);
    node_ptr eval_object_init(node_ptr& node);
    node_ptr eval_enum(node_ptr& node);
    node_ptr eval_union(node_ptr& node);
    node_ptr eval_try_catch(node_ptr& node);
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
    node_ptr eval_bit_and(node_ptr& node);
    node_ptr eval_bit_or(node_ptr& node);
    node_ptr eval_eq(node_ptr& node);
    node_ptr eval_dot(node_ptr& node);
    node_ptr eval_plus_eq(node_ptr& node);
    node_ptr eval_minus_eq(node_ptr& node);
    // Builtin functions
    void builtin_print(node_ptr& node);
    node_ptr eval_load_lib(node_ptr& node);
    node_ptr eval_call_lib_function(node_ptr& lib, node_ptr& node);

    Symbol new_symbol(std::string name, node_ptr& value, node_ptr type = nullptr);
    Symbol get_symbol(std::string name, sym_t_ptr& symbol_table);
    Symbol get_symbol_local(std::string& name, sym_t_ptr& symbol_table);
    void add_symbol(Symbol symbol, sym_t_ptr& symbol_table);
    void delete_symbol(std::string name, sym_t_ptr& symbol_table);
    bool match_types(node_ptr& _nodeA, node_ptr& _nodeB);
    bool match_values(node_ptr& nodeA, node_ptr& nodeB);

    void erase_prev();
    void erase_next();
    void erase_curr();

    node_ptr new_number_node(double value);
    node_ptr new_string_node(std::string value);
    node_ptr new_boolean_node(bool value);
    node_ptr new_accessor_node();
    node_ptr new_node(NodeType type);
    node_ptr new_node();

    std::string printable(node_ptr& node);

    void replaceAll(std::string& str, const std::string& from, const std::string& to);

    void error_and_exit(std::string message);
    node_ptr throw_error(std::string message);
};