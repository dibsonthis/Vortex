#include "Interpreter.hpp"

void Interpreter::advance(int n) {
    index += n;
    if (index < nodes.size()) {
        current_node = nodes[index];
        line = current_node->line;
        column = current_node->column;
    }
}

node_ptr Interpreter::peek(int n) {
    int idx = index + n;
    if (idx < nodes.size()) {
        return nodes[idx];
    }
}

void Interpreter::reset(int idx) {
    index = idx;
    current_node = nodes[index];
}

void Interpreter::eval_const_functions() {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::CONSTANT_DECLARATION && current_node->ConstantDeclaration.value->type == NodeType::FUNC) {
            Symbol symbol = new_symbol(current_node->ConstantDeclaration.name, current_node->ConstantDeclaration.value, true);
            add_symbol(symbol, symbol_table);
            erase_curr();
            continue;
        }

        advance();
    }
}

void Interpreter::eval_const_decl(node_ptr node) {
    Symbol symbol = new_symbol(node->ConstantDeclaration.name, node->ConstantDeclaration.value, true);
    add_symbol(symbol, symbol_table);
}

void Interpreter::eval_const_decl_multiple(node_ptr node) {
    for (node_ptr& decl : node->ConstantDeclarationMultiple.constant_declarations) {
        Symbol symbol = new_symbol(decl->ConstantDeclaration.name, decl->ConstantDeclaration.value, true);
        add_symbol(symbol, symbol_table);
    }
}

void Interpreter::eval_var_decl(node_ptr node) {
    Symbol symbol = new_symbol(node->VariableDeclaration.name, node->VariableDeclaration.value);
    add_symbol(symbol, symbol_table);
}

void Interpreter::eval_var_decl_multiple(node_ptr node) {
    for (node_ptr& decl : node->VariableDeclarationMultiple.variable_declarations) {
        Symbol symbol = new_symbol(decl->VariableDeclaration.name, decl->VariableDeclaration.value);
        add_symbol(symbol, symbol_table);
    }
}

void Interpreter::evaluate() {
    eval_const_functions();
    reset(0);

    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::CONSTANT_DECLARATION) {
            eval_const_decl(current_node);
            advance();
            continue;
        }
        if (current_node->type == NodeType::VARIABLE_DECLARATION) {
            eval_var_decl(current_node);
            advance();
            continue;
        }
        if (current_node->type == NodeType::CONSTANT_DECLARATION_MULTIPLE) {
            eval_const_decl_multiple(current_node);
            continue;
        }
        if (current_node->type == NodeType::VARIABLE_DECLARATION_MULTIPLE) {
            eval_var_decl_multiple(current_node);
            advance();
            continue;
        }

        advance();
    }
}

Symbol Interpreter::new_symbol(std::string name, node_ptr value, bool is_const, node_ptr type) {
    Symbol symbol;
    symbol.name = name;
    symbol.value = value;
    symbol.is_const = is_const;
    symbol.type = type;
    return symbol;
}

Symbol Interpreter::get_symbol(std::string name, std::shared_ptr<SymbolTable> symbol_table) {

}

void Interpreter::add_symbol(Symbol symbol, std::shared_ptr<SymbolTable> symbol_table) {
    symbol_table->symbols.push_back(symbol);
}

void Interpreter::erase_next() {
    nodes.erase(nodes.begin() + index + 1);
}

void Interpreter::erase_prev() {
    nodes.erase(nodes.begin() + index - 1);
    index--;
    current_node = nodes[index];
}

void Interpreter::erase_curr() {
    nodes.erase(nodes.begin() + index);
    index--;
    current_node = nodes[index];
}