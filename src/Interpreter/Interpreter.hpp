#pragma once

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
    std::shared_ptr<SymbolTable> parent = nullptr;
    std::shared_ptr<SymbolTable> child = nullptr;
};

class Interpreter {
public:
    std::vector<node_ptr> nodes;
    node_ptr current_node;
    int index = 0;
    std::string file_name;
    int line, column;
    std::shared_ptr<SymbolTable> symbol_table = std::make_shared<SymbolTable>();

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
    void eval_const_functions();
    void eval_const_decl(node_ptr node);
    void eval_const_decl_multiple(node_ptr node);
    void eval_var_decl(node_ptr node);
    void eval_var_decl_multiple(node_ptr node);

    Symbol new_symbol(std::string name, node_ptr value, bool is_const = false, node_ptr type = nullptr);
    Symbol get_symbol(std::string name, std::shared_ptr<SymbolTable> symbol_table);
    void add_symbol(Symbol symbol, std::shared_ptr<SymbolTable> symbol_table);

    void erase_prev();
    void erase_next();
    void erase_curr();
};