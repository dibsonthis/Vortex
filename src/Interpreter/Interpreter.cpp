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
    if (node->Object.elements[0]->type != NodeType::COMMA_LIST) {
        node_ptr prop = node->Object.elements[0];
        if (prop->Operator.value != ":") {
            error_and_exit("Object must contain properties separated with ':'");
        }
        if (prop->Operator.left->type != NodeType::ID) {
            error_and_exit("Propertiy names must be identifiers");
        }
        object->Object.properties[prop->Operator.left->ID.value] = eval_node(prop->Operator.right);
    }
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

node_ptr Interpreter::eval_func_call(node_ptr node, node_ptr func) {
    node_ptr function = new_node();
    function->type = NodeType::FUNC;

    if (node->FuncCall.name == "print") {
        if (node->FuncCall.args.size() == 1) {
            builtin_print(eval_node(node->FuncCall.args[0]));
        } else {
            for (node_ptr arg : node->FuncCall.args) {
                builtin_print(eval_node(arg));
                std::cout << '\n';
            }
        }
        return new_node(NodeType::NONE);
    }
    if (node->FuncCall.name == "println") {
        if (node->FuncCall.args.size() == 1) {
            builtin_print(eval_node(node->FuncCall.args[0]));
            std::cout << "\n";
        } else {
            for (node_ptr arg : node->FuncCall.args) {
                builtin_print(eval_node(arg));
                std::cout << '\n';
            }
        }
        return new_node(NodeType::NONE);
    }
    if (node->FuncCall.name == "string") {
        if (node->FuncCall.args.size() != 1) {
            error_and_exit("Function " + node->FuncCall.name + " expects 1 argument");
        }
        return new_string_node(printable(eval_node(node->FuncCall.args[0])));
    }

    if (func != nullptr) {
        function->Function.name = func->Function.name;
        function->Function.args = std::vector<node_ptr>(func->Function.args);
        function->Function.params = std::vector<node_ptr>(func->Function.params);
        function->Function.body = func->Function.body;
    } else if (node->FuncCall.caller == nullptr) {
        Symbol function_symbol = get_symbol(node->FuncCall.name, symbol_table);
        if (function_symbol.value == nullptr) {
            error_and_exit("Function '" + node->FuncCall.name + "' is undefined");
        }
        function->Function.name = function_symbol.name;
        function->Function.args = std::vector<node_ptr>(function_symbol.value->Function.args);
        function->Function.params = std::vector<node_ptr>(function_symbol.value->Function.params);
        function->Function.body = function_symbol.value->Function.body;
    } else {
        node_ptr method = node->FuncCall.caller->Object.properties[node->FuncCall.name];
        if (method->type == NodeType::NONE) {
            error_and_exit("Method '" + node->FuncCall.name + "' does not exist");
        }
        function->Function.name = method->Function.name;
        function->Function.args = std::vector<node_ptr>(method->Function.args);
        function->Function.params = std::vector<node_ptr>(method->Function.params);
        function->Function.body = method->Function.body;
    }

    std::vector<node_ptr> args;
    for (node_ptr arg : node->FuncCall.args) {
        args.push_back(eval_node(arg));
    }
    current_symbol_table->child = std::make_shared<SymbolTable>();
    auto function_symbol_table = current_symbol_table->child;
    function_symbol_table->parent = current_symbol_table;
    function_symbol_table->symbols = current_symbol_table->symbols;
    if (node->FuncCall.caller != nullptr) {
        function_symbol_table->symbols["this"] = new_symbol("this", node->FuncCall.caller);
    }

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
        if (function->Function.body->type != NodeType::OBJECT) {
            res = eval_node(function->Function.body);
        } else {
            for (int i = 0; i < function->Function.body->Object.elements.size(); i++) {
                node_ptr expr = function->Function.body->Object.elements[i];
                node_ptr evaluated_expr = eval_node(expr);
                if (evaluated_expr->type == NodeType::RETURN) {
                    res = evaluated_expr->Return.value;
                    break;
                } else if (i == function->Function.body->Object.elements.size()-1) {
                    res = evaluated_expr;
                    if (res->type == NodeType::RETURN) {
                        res = res->Return.value;
                    }
                    break;
                }
            }
        }
    }

    current_symbol_table = current_symbol_table->parent;

    return res;
}

node_ptr Interpreter::eval_if_statement(node_ptr node) {
    node_ptr conditional = eval_node(node->IfStatement.condition);

    if (conditional->type != NodeType::BOOLEAN) {
        error_and_exit("If statement conditional must evaluate to a boolean");
    }

    if (conditional->Boolean.value) {
        for (node_ptr expr : node->IfStatement.body->Object.elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                return evaluated_expr;
            }
        }
    }

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_if_block(node_ptr node) {
    for (node_ptr statement : node->IfBlock.statements) {
        if (statement->type == NodeType::IF_STATEMENT) {
            node_ptr conditional = eval_node(statement->IfStatement.condition);
            if (conditional->Boolean.value) {
                return eval_node(statement);
            }
        } else if (statement->type == NodeType::OBJECT) {
            for (node_ptr expr : statement->Object.elements) {
                if (expr->type == NodeType::RETURN) {
                    return eval_node(expr);
                }
                eval_node(expr);
            }
            return new_node(NodeType::NONE);
        }
    }

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_return(node_ptr node) {
    node_ptr ret = new_node();
    ret->type = NodeType::RETURN;
    ret->Return.value = eval_node(node->Return.value);
    return ret;
}

node_ptr Interpreter::eval_for_loop(node_ptr node) {

    if (node->ForLoop.iterator != nullptr) {
        node_ptr iterator = eval_node(node->ForLoop.iterator);
        
        for (int i = 0; i < iterator->List.elements.size(); i++) {
            if (node->ForLoop.index_name) {
                add_symbol(new_symbol(node->ForLoop.index_name->ID.value, new_number_node(i)), current_symbol_table);
            }
            if (node->ForLoop.value_name) {
                add_symbol(new_symbol(node->ForLoop.value_name->ID.value, iterator->List.elements[i]), current_symbol_table);
            }
            for (node_ptr expr : node->ForLoop.body->Object.elements) {
                if (expr->ID.value == "break") {
                    goto _break;
                }
                if (expr->ID.value == "continue") {
                    goto _continue;
                }
                node_ptr evaluated_expr = eval_node(expr);
                if (evaluated_expr->type == NodeType::RETURN) {
                    return evaluated_expr;
                }
            }

            _continue:
                continue;
            _break:
                break;
        }

        // Cleanup

        if (node->ForLoop.index_name != nullptr) {
            delete_symbol(node->ForLoop.index_name->ID.value, current_symbol_table);
        }
        if (node->ForLoop.value_name != nullptr) {
            delete_symbol(node->ForLoop.value_name->ID.value, current_symbol_table);
        }

        return new_node(NodeType::NONE);
    }
    
    for (int i = node->ForLoop.start->Number.value; i < node->ForLoop.end->Number.value; i++) {
        int index = i-node->ForLoop.start->Number.value;
        if (node->ForLoop.index_name) {
            add_symbol(new_symbol(node->ForLoop.index_name->ID.value, new_number_node(index)), current_symbol_table);
        }
        if (node->ForLoop.value_name) {
        add_symbol(new_symbol(
            node->ForLoop.value_name->ID.value, new_number_node(i)), current_symbol_table);
        }
        for (node_ptr expr : node->ForLoop.body->Object.elements) {
            if (expr->ID.value == "break") {
                goto _break2;
            }
            if (expr->ID.value == "continue") {
                goto _continue2;
            }
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                return evaluated_expr;
            }
        }

        _continue2:
            continue;
        _break2:
            break;
    }

    // Cleanup

    if (node->ForLoop.index_name != nullptr) {
        delete_symbol(node->ForLoop.index_name->ID.value, current_symbol_table);
    }
    if (node->ForLoop.value_name != nullptr) {
        delete_symbol(node->ForLoop.value_name->ID.value, current_symbol_table);
    }

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_while_loop(node_ptr node) {
    node_ptr conditional = eval_node(node->WhileLoop.condition);
    if (conditional->type != NodeType::BOOLEAN) {
        error_and_exit("While loop conditional must evaluate to a boolean");
    }

    while (conditional->Boolean.value) {
        for (node_ptr expr : node->WhileLoop.body->Object.elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                return evaluated_expr->Return.value;
            }
        }
        conditional = eval_node(node->WhileLoop.condition);
    }

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_accessor(node_ptr node) {
    node_ptr container = eval_node(node->Accessor.container);
    node_ptr accessor = eval_node(node->Accessor.accessor);

    if (accessor->List.elements.size() != 1) {
        error_and_exit("Malformed accessor");
    }

    if (container->type == NodeType::LIST) {
        node_ptr index_node = accessor->List.elements[0];
        if (index_node->type != NodeType::NUMBER) {
            error_and_exit("List accessor expects a number");
        }
        int index = index_node->Number.value;
        if (index >= container->List.elements.size() || index < 0) {
            return new_node(NodeType::NONE);
        }
        return container->List.elements[index];
    }

    if (container->type == NodeType::OBJECT) {
        node_ptr prop_node = accessor->List.elements[0];
        if (prop_node->type != NodeType::STRING) {
            error_and_exit("Object accessor expects a string");
        }
        std::string prop = prop_node->String.value;
        if (container->Object.properties.contains(prop)) {
            return container->Object.properties[prop];
        }
        return new_node(NodeType::NONE);
    }

    error_and_exit("Value of type '" + node_repr(container) + "' is not accessable");
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

node_ptr Interpreter::eval_dot(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = node->Operator.right;

    if (left->type != NodeType::OBJECT) {
        error_and_exit("Left hand side of '.' must be an object");
    }

    if (right->type != NodeType::ID && right->type != NodeType::FUNC_CALL) {
        error_and_exit("Right hand side of '.' must be an identifier or function call");
    }

    if (right->type == NodeType::ID) {
        if (left->Object.properties.contains(right->ID.value)) {
            return left->Object.properties[right->ID.value];
        }
        return new_node(NodeType::NONE);
    }

    if (right->type == NodeType::FUNC_CALL) {
        right->FuncCall.caller = left;
        return eval_func_call(right);
    }
}

node_ptr Interpreter::eval_eq(node_ptr node) {
    node_ptr right = eval_node(node->Operator.right);
    node_ptr left = node->Operator.left;

    if (left->Operator.value == ".") {
        node_ptr object = left->Operator.left;
        if (object->type == NodeType::ID) {
            Symbol symbol = get_symbol(object->ID.value, current_symbol_table);
            if (symbol.is_const) {
                error_and_exit("Cannot modify constant object '" + symbol.name + "'");
            }
        }
        object = eval_node(left->Operator.left);
        node_ptr prop = left->Operator.right;

        if (object->type != NodeType::OBJECT) {
            error_and_exit("Left hand side of '.' must be an object");
        }

        if (prop->type != NodeType::ID) {
            error_and_exit("Right hand side of '.' must be an identifier");
        }

        object->Object.properties[prop->ID.value] = right;
        return object;
    }

    if (left->type == NodeType::ID) {
        Symbol symbol = get_symbol(left->ID.value, current_symbol_table);
        if (symbol.value != nullptr) {
            // Re-assigning, check if const
            if (symbol.is_const) {
                error_and_exit("Cannot modify constant '" + symbol.name + "'");
            }

            // In case we need to feed this back to the onChange functions

            node_ptr old_value = std::make_shared<Node>(*symbol.value);

            *symbol.value = *right;
            
            // Call onChange functions

            for (node_ptr function : symbol.onChangeFunctions) {
                node_ptr function_call = new_node(NodeType::FUNC_CALL);
                function_call->FuncCall.name = function->Function.name;
                function_call->FuncCall.args = std::vector<node_ptr>();
                if (function->Function.params.size() > 0) {
                    function_call->FuncCall.args.push_back(symbol.value);
                }
                if (function->Function.params.size() > 1) {
                    function_call->FuncCall.args.push_back(old_value);
                }
                if (function->Function.params.size() > 2) {
                    function_call->FuncCall.args.push_back(new_string_node(symbol.name));
                }
                if (function->Function.params.size() > 3) {
                    node_ptr file_info = new_node(NodeType::OBJECT);
                    file_info->Object.properties["filename"] = new_string_node(file_name);
                    file_info->Object.properties["line"] = new_number_node(line);
                    file_info->Object.properties["column"] = new_number_node(column);
                    function_call->FuncCall.args.push_back(file_info);
                }
                eval_func_call(function_call, function);
            }

            return right;
        }

        error_and_exit("Variable '" + left->ID.value + "' is undefined");
    }

    if (left->Operator.value == "::") {
        node_ptr target = left->Operator.left;
        node_ptr prop = left->Operator.right;

        if (target->type != NodeType::ID && target->type != NodeType::LIST) {
            error_and_exit("Hook expects either an identifier or a list of identifiers");
        }

        if (prop->type != NodeType::ID) {
            error_and_exit("Hook expects an identifier");
        }

        if (target->type == NodeType::ID) {
            Symbol symbol = get_symbol(target->ID.value, current_symbol_table);

            if (prop->ID.value == "onChange") {
                if (right->type != NodeType::FUNC) {
                    error_and_exit("onChange hook expects a function");
                }
                symbol.onChangeFunctions.push_back(right);
                add_symbol(symbol, current_symbol_table);
            }
        } else {
            if (target->List.elements.size() == 1 && target->List.elements[0]->type == NodeType::COMMA_LIST) {
                target = target->List.elements[0];
            }
            for (node_ptr elem : target->List.elements) {
                if (elem->type != NodeType::ID) {
                    error_and_exit("Hook expects either an identifier or a list of identifiers");
                }

                Symbol symbol = get_symbol(elem->ID.value, current_symbol_table);

                if (prop->ID.value == "onChange") {
                    if (right->type != NodeType::FUNC) {
                        error_and_exit("onChange hook expects a function");
                    }
                    symbol.onChangeFunctions.push_back(right);
                    add_symbol(symbol, current_symbol_table);
                }
            }
        }

        return new_node(NodeType::NONE);
    }
}

// Builtin functions

std::string Interpreter::printable(node_ptr node) {
    switch (node->type) {
        case NodeType::NUMBER: {
            return std::to_string(node->Number.value);
        }
        case NodeType::BOOLEAN: {
            return node->Boolean.value ? "true" : "false";
        }
        case NodeType::STRING: {
            return node->String.value;
        }
        case NodeType::FUNC: {
            std::string res = "( ";
            for (node_ptr param : node->Function.params) {
                res += param->ID.value + " ";
            }
            res += ") => ...";
            return res;
        }
        case NodeType::LIST: {
            std::string res = "[ ";
            for (node_ptr elem : node->List.elements) {
                res += printable(node) + " ";
            }
            res += "]";
            return res;
        }
        case NodeType::OBJECT: {
            std::string res = "{\n\n";
            for (auto const& elem : node->Object.properties) {
                res += elem.first + ": " + printable(elem.second) + '\n';
            }
            res += "}";
            return res;
        }
        case NodeType::NONE: {
            return "None";
        }
        default: {
            return "<not implemented>";
        }
    }
}

void Interpreter::builtin_print(node_ptr node) {
    std::cout << printable(node) << std::flush;
}

node_ptr Interpreter::eval_node(node_ptr node) {
    line = node->line;
    column = node->column;

    if (node->type == NodeType::NUMBER 
    || node->type == NodeType::STRING 
    || node->type == NodeType::BOOLEAN) {
        return node;
    }
    if (node->type == NodeType::LIST) {
        return eval_list(node);
    }
    if (node->type == NodeType::OBJECT) {
        if (node->Object.elements.size() == 1 && (node->Object.elements[0]->type == NodeType::COMMA_LIST || node->Object.elements[0]->Operator.value == ":")) {
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
    if (node->type == NodeType::IF_STATEMENT) {
        return eval_if_statement(node);
    }
    if (node->type == NodeType::IF_BLOCK) {
        return eval_if_block(node);
    }
    if (node->type == NodeType::WHILE_LOOP) {
        return eval_while_loop(node);
    }
    if (node->type == NodeType::FOR_LOOP) {
        return eval_for_loop(node);
    }
    if (node->type == NodeType::ACCESSOR) {
        return eval_accessor(node);
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
    if (node->Operator.value == ".") {
        return eval_dot(node);
    }
    if (node->Operator.value == "=") {
        return eval_eq(node);
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
    sym_t_ptr scope = symbol_table;

    if (scope->symbols.contains(name)) {
        return scope->symbols[name];
    }
    while (scope->parent != nullptr) {
        scope = scope->parent;
        if (scope->symbols.contains(name)) {
            return scope->symbols[name];
        }
    }

    return new_symbol("_undefined_", nullptr);
}

void Interpreter::add_symbol(Symbol symbol, std::shared_ptr<SymbolTable> symbol_table) {
    symbol_table->symbols[symbol.name] = symbol;
}

void Interpreter::delete_symbol(std::string name, std::shared_ptr<SymbolTable> symbol_table) {
    symbol_table->symbols.erase(name);
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

node_ptr Interpreter::new_node(NodeType type) {
    auto node = std::make_shared<Node>(type);
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