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

node_ptr Interpreter::eval_const_decl(node_ptr node) {
    Symbol symbol = new_symbol(node->ConstantDeclaration.name, eval_node(node->ConstantDeclaration.value), true);
    add_symbol(symbol, symbol_table);
    return node;
}

node_ptr Interpreter::eval_const_decl_multiple(node_ptr node) {
    for (node_ptr& decl : node->ConstantDeclarationMultiple.constant_declarations) {
        Symbol symbol = new_symbol(decl->ConstantDeclaration.name, eval_node(decl->ConstantDeclaration.value), true);
        add_symbol(symbol, symbol_table);
    }
    return node;
}

node_ptr Interpreter::eval_var_decl(node_ptr node) {
    Symbol symbol = new_symbol(node->VariableDeclaration.name, eval_node(node->VariableDeclaration.value));
    add_symbol(symbol, symbol_table);
    return node;
}

node_ptr Interpreter::eval_var_decl_multiple(node_ptr node) {
    for (node_ptr& decl : node->VariableDeclarationMultiple.variable_declarations) {
        Symbol symbol = new_symbol(decl->VariableDeclaration.name, eval_node(decl->VariableDeclaration.value));
        add_symbol(symbol, symbol_table);
    }
    return node;
}

// Operations

node_ptr Interpreter::eval_pos_neg(node_ptr node) {
    node_ptr value = eval_node(node->Operator.right);
    if (value->type != NodeType::NUMBER) {
        error_and_exit("Cannot negate a non-number");
    }
    if (node->Operator.value == "-") {
        return new_number_node(-value->Number.value);
    } else {
        return new_number_node(+value->Number.value);
    }

    return node;
}

node_ptr Interpreter::eval_plus(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->Number.value + right->Number.value);
    }

    if (left->type == NodeType::STRING && right->type == NodeType::STRING) {
        return new_string_node(left->String.value + right->String.value);
    }

    error_and_exit("Cannot perform operation '+' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_minus(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->Number.value - right->Number.value);
    }

    error_and_exit("Cannot perform operation '-' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_node(node_ptr node) {
    if (node->type == NodeType::NUMBER 
    || node->type == NodeType::STRING 
    || node->type == NodeType::BOOLEAN) {
        return node;
    }
    if (node->type == NodeType::ID) {
        return get_symbol(node->ID.value, symbol_table).value;
    }
    if (node->type == NodeType::CONSTANT_DECLARATION) {
        return eval_const_decl(node);
    }
    if (node->type == NodeType::VARIABLE_DECLARATION) {
        return eval_var_decl(node);
    }
    if (node->type == NodeType::CONSTANT_DECLARATION_MULTIPLE) {
        return eval_const_decl_multiple(node);
    }
    if (node->type == NodeType::VARIABLE_DECLARATION_MULTIPLE) {
        return eval_var_decl_multiple(node);
    }
    if ((node->Operator.value == "+" || node->Operator.value == "-") 
        && node->Operator.left == nullptr) {
        return eval_pos_neg(node);
    }
    if (node->Operator.value == "+") {
        return eval_plus(node);
    }
    if (node->Operator.value == "-") {
        return eval_minus(node);
    }

    return node;
}

void Interpreter::evaluate() {
    eval_const_functions();
    reset(0);

    while (current_node->type != NodeType::END_OF_FILE) {
        eval_node(current_node);
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
    // TODO
    // Temp solution, will change to better approach
    // Or change symbol table to be a map
    for (auto& symbol : symbol_table->symbols) {
        if (symbol.name == name) {
            return symbol;
        }
    }

    return new_symbol("_undefined_", nullptr);
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

node_ptr Interpreter::new_number_node(double value) {
    auto node = std::make_shared<Node>(NodeType::NUMBER);
    node->Number.value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_string_node(std::string value) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->String.value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_boolean_node(bool value) {
    auto node = std::make_shared<Node>(NodeType::BOOLEAN);
    node->Boolean.value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_accessor_node() {
    auto node = std::make_shared<Node>(NodeType::ACCESSOR);
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_node() {
    auto node = std::make_shared<Node>();
    node->line = line;
    node->column = column;
    return node;
}

void Interpreter::error_and_exit(std::string message)
{
    std::string error_message = "Error in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message << "\n";
    exit(1);
}