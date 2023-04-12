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
            node_ptr existing_symbol = get_symbol(current_node->ConstantDeclaration.name, symbol_table).value;
            if (existing_symbol != nullptr) {
                error_and_exit("Function '" + current_node->ConstantDeclaration.name + "' is already defined");
            }
            Symbol symbol = new_symbol(current_node->ConstantDeclaration.name, current_node->ConstantDeclaration.value, true);
            add_symbol(symbol, current_symbol_table);
            erase_curr();
            continue;
        }

        advance();
    }
}

node_ptr Interpreter::eval_const_decl(node_ptr node) {
    node_ptr existing_symbol = get_symbol(node->ConstantDeclaration.name, symbol_table).value;
    if (existing_symbol != nullptr) {
        error_and_exit("Variable '" + node->ConstantDeclaration.name + "' is already defined");
    }
    Symbol symbol = new_symbol(node->ConstantDeclaration.name, eval_node(node->ConstantDeclaration.value), true);
    add_symbol(symbol, current_symbol_table);
    return symbol.value;
}

node_ptr Interpreter::eval_const_decl_multiple(node_ptr node) {
    for (node_ptr& decl : node->ConstantDeclarationMultiple.constant_declarations) {
        node_ptr existing_symbol = get_symbol(decl->ConstantDeclaration.name, symbol_table).value;
        if (existing_symbol != nullptr) {
            error_and_exit("Variable '" + decl->ConstantDeclaration.name + "' is already defined");
        }
        Symbol symbol = new_symbol(decl->ConstantDeclaration.name, eval_node(decl->ConstantDeclaration.value), true);
        add_symbol(symbol, current_symbol_table);
    }
    return new_boolean_node(true);
}

node_ptr Interpreter::eval_var_decl(node_ptr node) {
    node_ptr existing_symbol = get_symbol(node->VariableDeclaration.name, symbol_table).value;
    if (existing_symbol != nullptr) {
        error_and_exit("Variable '" + node->VariableDeclaration.name + "' is already defined");
    }
    Symbol symbol = new_symbol(node->VariableDeclaration.name, eval_node(node->VariableDeclaration.value));
    add_symbol(symbol, current_symbol_table);
    return symbol.value;
}

node_ptr Interpreter::eval_var_decl_multiple(node_ptr node) {
    for (node_ptr& decl : node->VariableDeclarationMultiple.variable_declarations) {
        node_ptr existing_symbol = get_symbol(decl->VariableDeclaration.name, symbol_table).value;
        if (existing_symbol != nullptr) {
            error_and_exit("Variable '" + decl->VariableDeclaration.name + "' is already defined");
        }
        Symbol symbol = new_symbol(decl->VariableDeclaration.name, eval_node(decl->VariableDeclaration.value));
        add_symbol(symbol, current_symbol_table);
    }
    return new_boolean_node(true);
}

node_ptr Interpreter::eval_list(node_ptr node) {
    node_ptr list = new_node();
    list->type = NodeType::LIST;
    if (node->List.elements.size() == 1) {
        if (node->List.elements[0]->type == NodeType::COMMA_LIST) {
            for (auto elem : node->List.elements[0]->List.elements) {
                list->List.elements.push_back(eval_node(elem));
            }
        } else {
            list->List.elements.push_back(eval_node(node->List.elements[0]));
        }
    }
    return list;
}

node_ptr Interpreter::eval_object(node_ptr node) {
    node_ptr object = new_node();
    object->type = NodeType::OBJECT;
    for (node_ptr prop : node->Object.elements[0]->List.elements) {
        if (prop->Operator.value != ":") {
            error_and_exit("Object must contain properties separated with ':'");
        }
        if (prop->Operator.left->type != NodeType::ID) {
            error_and_exit("Propertiy names must be identifiers");
        }
        object->Object.properties[prop->Operator.left->ID.value] = eval_node(prop->Operator.right);
    }
    return object;
}

node_ptr Interpreter::eval_func_call(node_ptr node) {
    Symbol function_symbol = get_symbol(node->FuncCall.name, symbol_table);
    if (function_symbol.value == nullptr) {
        error_and_exit("Function '" + node->FuncCall.name + "' is undefined");
    }
    node_ptr function = new_node();
    function->type = NodeType::FUNC;
    function->Function.name = function_symbol.name;
    function->Function.args = std::vector<node_ptr>(function_symbol.value->Function.args);
    function->Function.params = std::vector<node_ptr>(function_symbol.value->Function.params);
    function->Function.body = function_symbol.value->Function.body;

    std::vector<node_ptr> args;
    for (node_ptr arg : node->FuncCall.args) {
        args.push_back(eval_node(arg));
    }
    current_symbol_table->child = std::make_shared<SymbolTable>();
    auto function_symbol_table = current_symbol_table->child;
    function_symbol_table->parent = current_symbol_table;
    function_symbol_table->symbols = current_symbol_table->symbols;

    current_symbol_table = function_symbol_table;

    int num_empty_args = std::count(function->Function.args.begin(), function->Function.args.end(), nullptr);

    if (args.size() > num_empty_args) {
        error_and_exit("Function '" + node->FuncCall.name + "' expects " + std::to_string(num_empty_args) + " parameters but " + std::to_string(args.size()) + " were provided");
    }

    int start_index = 0;

    for (node_ptr arg : function->Function.args) {
        if (arg == nullptr) {
            break;
        }
        start_index++;
    }

    for (int i = 0; i < function->Function.args.size(); i++) {
        if (function->Function.args[i] != nullptr) {
            std::string name = function->Function.params[i]->ID.value;
            node_ptr value = function->Function.args[i];
            Symbol symbol = new_symbol(name, value);
            add_symbol(symbol, function_symbol_table);
        }
    }

    for (int i = 0; i < args.size(); i++) {
        std::string name = function->Function.params[i+start_index]->ID.value;
        node_ptr value = args[i];
        Symbol symbol = new_symbol(name, value);
        add_symbol(symbol, function_symbol_table);
        function->Function.args[i+start_index] = value;
    }

    node_ptr res = function;

    // Check if function's args vector has any nullptrs
    // If it does, it's curried, else we run the function

    if (std::find(function->Function.args.begin(), function->Function.args.end(), nullptr) == function->Function.args.end()) {
        res = eval_node(function->Function.body);
    }

    current_symbol_table = current_symbol_table->parent;

    return res;
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

node_ptr Interpreter::eval_add(node_ptr node) {
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

node_ptr Interpreter::eval_sub(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->Number.value - right->Number.value);
    }

    error_and_exit("Cannot perform operation '-' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_mul(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->Number.value * right->Number.value);
    }

    error_and_exit("Cannot perform operation '*' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_div(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->Number.value / right->Number.value);
    }

    error_and_exit("Cannot perform operation '/' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_pow(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(pow(left->Number.value, right->Number.value));
    }

    error_and_exit("Cannot perform operation '^' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_eq_eq(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type != right->type) {
        return new_boolean_node(false);
    }

    if (left->type == NodeType::NUMBER) {
        return new_boolean_node(left->Number.value == right->Number.value);
    }

    if (left->type == NodeType::STRING) {
        return new_boolean_node(left->String.value == right->String.value);
    }

    if (left->type == NodeType::BOOLEAN) {
        return new_boolean_node(left->Boolean.value == right->Boolean.value);
    }

    // TODO: Extend to lists and objects

    return new_boolean_node(false);
}

node_ptr Interpreter::eval_not_eq(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type != right->type) {
        return new_boolean_node(true);
    }

    if (left->type == NodeType::NUMBER) {
        return new_boolean_node(left->Number.value != right->Number.value);
    }

    if (left->type == NodeType::STRING) {
        return new_boolean_node(left->String.value != right->String.value);
    }

    if (left->type == NodeType::BOOLEAN) {
        return new_boolean_node(left->Boolean.value != right->Boolean.value);
    }

    // TODO: Extend to lists and objects

    return new_boolean_node(true);
}

node_ptr Interpreter::eval_lt_eq(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->Number.value <= right->Number.value);
    }

    error_and_exit("Cannot perform operation '<=' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_gt_eq(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->Number.value >= right->Number.value);
    }

    error_and_exit("Cannot perform operation '>=' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_lt(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->Number.value < right->Number.value);
    }

    error_and_exit("Cannot perform operation '<' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_gt(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->Number.value > right->Number.value);
    }

    error_and_exit("Cannot perform operation '>' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_and(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);

    if (left->type == NodeType::BOOLEAN && !left->Boolean.value) {
        return left;
    }

    node_ptr right = eval_node(node->Operator.right);

    if (right->type == NodeType::BOOLEAN && !right->Boolean.value) {
        return right;
    }

    return left;

    error_and_exit("Cannot perform operation '&&' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_or(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);

    if (left->type == NodeType::BOOLEAN && left->Boolean.value) {
        return left;
    }

    node_ptr right = eval_node(node->Operator.right);

    if (left->type == NodeType::BOOLEAN && right->type == NodeType::BOOLEAN) {
        return new_boolean_node(left->Boolean.value || right->Boolean.value);
    }

    error_and_exit("Cannot perform operation '||' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_node(node_ptr node) {
    if (node->type == NodeType::NUMBER 
    || node->type == NodeType::STRING 
    || node->type == NodeType::BOOLEAN) {
        return node;
    }
    if (node->type == NodeType::LIST) {
        return eval_list(node);
    }
    if (node->type == NodeType::OBJECT) {
        if (node->Object.elements.size() == 1 && node->Object.elements[0]->type == NodeType::COMMA_LIST) {
            return eval_object(node);
        }
    }
    if (node->type == NodeType::ID) {
        node_ptr value = get_symbol(node->ID.value, current_symbol_table).value;
        if (value == nullptr) {
            error_and_exit("Variable '" + node->ID.value + "' is undefined");
        }
        return value;
    }
    if (node->type == NodeType::PAREN) {
        if (node->Paren.elements.size() != 1) {
            error_and_exit("Empty parentheses");
        }
        return eval_node(node->Paren.elements[0]);
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
    if (node->type == NodeType::FUNC_CALL) {
        return eval_func_call(node);
    }
    if ((node->Operator.value == "+" || node->Operator.value == "-") 
        && node->Operator.left == nullptr) {
        return eval_pos_neg(node);
    }
    if (node->Operator.value == "+") {
        return eval_add(node);
    }
    if (node->Operator.value == "-") {
        return eval_sub(node);
    }
    if (node->Operator.value == "*") {
        return eval_mul(node);
    }
    if (node->Operator.value == "/") {
        return eval_div(node);
    }
    if (node->Operator.value == "^") {
        return eval_pow(node);
    }
    if (node->Operator.value == "==") {
        return eval_eq_eq(node);
    }
    if (node->Operator.value == "!=") {
        return eval_not_eq(node);
    }
    if (node->Operator.value == "<=") {
        return eval_lt_eq(node);
    }
    if (node->Operator.value == ">=") {
        return eval_gt_eq(node);
    }
    if (node->Operator.value == "<") {
        return eval_lt(node);
    }
    if (node->Operator.value == ">") {
        return eval_gt(node);
    }
    if (node->Operator.value == "&&") {
        return eval_and(node);
    }
    if (node->Operator.value == "||") {
        return eval_or(node);
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