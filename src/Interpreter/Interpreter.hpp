#pragma once

#include "../Node/Node.hpp"
#include "../utils/utils.hpp"
struct Symbol {
    node_ptr value;
    node_ptr type;
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
    std::shared_ptr<SymbolTable> symbol_table;

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

    Symbol new_symbol(std::string name, node_ptr type = nullptr);
    Symbol get_symbol(std::string name, std::shared_ptr<SymbolTable> symbol_table);
    void add_symbol(Symbol symbol, std::shared_ptr<SymbolTable> symbol_table);
};