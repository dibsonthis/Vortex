#pragma once

#define sym_t_ptr std::shared_ptr<SymbolTable>

#include <cmath>
#include "../Node/Node.hpp"
#include "../utils/utils.hpp"
struct Symbol {
    std::string name;
    node_ptr value;
    node_ptr type;
    bool is_const;
    std::vector<node_ptr> onChangeFunctions = std::vector<node_ptr>();
};

struct SymbolTable {
    std::vector<Symbol> symbols;
    sym_t_ptr parent = nullptr;
    sym_t_ptr child = nullptr;
};

class Interpreter {
public:
    std::vector<node_ptr> nodes;
    node_ptr current_node;
    int index = 0;
    std::string file_name;
    int line, column;
    sym_t_ptr symbol_table = std::make_shared<SymbolTable>();
    sym_t_ptr current_symbol_table = symbol_table;

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
    node_ptr eval_node(node_ptr node);
    void eval_const_functions();
    node_ptr eval_const_decl(node_ptr node);
    node_ptr eval_const_decl_multiple(node_ptr node);
    node_ptr eval_var_decl(node_ptr node);
    node_ptr eval_var_decl_multiple(node_ptr node);
    node_ptr eval_list(node_ptr node);
    node_ptr eval_object(node_ptr node);
    node_ptr eval_func_call(node_ptr node);
    node_ptr eval_if_statement(node_ptr node);
    node_ptr eval_if_block(node_ptr node);
    // Operations
    node_ptr eval_pos_neg(node_ptr node);
    node_ptr eval_add(node_ptr node);
    node_ptr eval_sub(node_ptr node);
    node_ptr eval_mul(node_ptr node);
    node_ptr eval_div(node_ptr node);
    node_ptr eval_pow(node_ptr node);
    node_ptr eval_eq_eq(node_ptr node);
    node_ptr eval_not_eq(node_ptr node);
    node_ptr eval_lt_eq(node_ptr node);
    node_ptr eval_gt_eq(node_ptr node);
    node_ptr eval_lt(node_ptr node);
    node_ptr eval_gt(node_ptr node);
    node_ptr eval_and(node_ptr node);
    node_ptr eval_or(node_ptr node);

    Symbol new_symbol(std::string name, node_ptr value, bool is_const = false, node_ptr type = nullptr);
    Symbol get_symbol(std::string name, sym_t_ptr symbol_table);
    void add_symbol(Symbol symbol, sym_t_ptr symbol_table);

    void erase_prev();
    void erase_next();
    void erase_curr();

    node_ptr new_number_node(double value);
    node_ptr new_string_node(std::string value);
    node_ptr new_boolean_node(bool value);
    node_ptr new_accessor_node();
    node_ptr new_node();

    void error_and_exit(std::string message);
};