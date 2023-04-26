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
            node_ptr existing_symbol = get_symbol(current_node->ConstantDeclaration.name, current_symbol_table).value;
            if (existing_symbol != nullptr) {
                error_and_exit("Function '" + current_node->ConstantDeclaration.name + "' is already defined");
            }
            current_node->ConstantDeclaration.value->Meta.is_const = true;
            current_node->ConstantDeclaration.value->Function.decl_filename = file_name;
            Symbol symbol = new_symbol(current_node->ConstantDeclaration.name, current_node->ConstantDeclaration.value);
            add_symbol(symbol, current_symbol_table);
            erase_curr();
            continue;
        }

        advance();
    }
}

node_ptr Interpreter::eval_const_decl(node_ptr node) {
    node_ptr existing_symbol = get_symbol_local(node->ConstantDeclaration.name, current_symbol_table).value;
    if (existing_symbol != nullptr) {
        error_and_exit("Variable '" + node->ConstantDeclaration.name + "' is already defined");
    }
    node_ptr value = eval_node(node->ConstantDeclaration.value);
    value = std::make_shared<Node>(*value);
    value->Meta.is_const = true;
    if (value->type == NodeType::HOOK) {
        value->Hook.name = node->ConstantDeclaration.name;
    }
    Symbol symbol = new_symbol(node->ConstantDeclaration.name, value);
    add_symbol(symbol, current_symbol_table);
    return symbol.value;
}

node_ptr Interpreter::eval_const_decl_multiple(node_ptr node) {
    for (node_ptr& decl : node->ConstantDeclarationMultiple.constant_declarations) {
        node_ptr existing_symbol = get_symbol_local(decl->ConstantDeclaration.name, current_symbol_table).value;
        if (existing_symbol != nullptr) {
            error_and_exit("Variable '" + decl->ConstantDeclaration.name + "' is already defined");
        }
        node_ptr value = eval_node(decl->ConstantDeclaration.value);
        value = std::make_shared<Node>(*value);
        value->Meta.is_const = true;
        if (value->type == NodeType::HOOK) {
            value->Hook.name = decl->ConstantDeclaration.name;
        }
        Symbol symbol = new_symbol(decl->ConstantDeclaration.name, value);
        add_symbol(symbol, current_symbol_table);
    }
    return new_boolean_node(true);
}

node_ptr Interpreter::eval_var_decl(node_ptr node) {
    node_ptr existing_symbol = get_symbol_local(node->VariableDeclaration.name, current_symbol_table).value;
    if (existing_symbol != nullptr) {
        error_and_exit("Variable '" + node->VariableDeclaration.name + "' is already defined");
    }
    node_ptr value = eval_node(node->VariableDeclaration.value);
    value = std::make_shared<Node>(*value);
    value->Meta.is_const = false;
    if (value->type == NodeType::HOOK) {
        value->Hook.name = node->VariableDeclaration.name;
    }
    Symbol symbol = new_symbol(node->VariableDeclaration.name, value);
    add_symbol(symbol, current_symbol_table);
    return symbol.value;
}

node_ptr Interpreter::eval_var_decl_multiple(node_ptr node) {
    for (node_ptr& decl : node->VariableDeclarationMultiple.variable_declarations) {
        node_ptr existing_symbol = get_symbol_local(decl->VariableDeclaration.name, current_symbol_table).value;
        if (existing_symbol != nullptr) {
            error_and_exit("Variable '" + decl->VariableDeclaration.name + "' is already defined");
        }
        node_ptr value = eval_node(decl->VariableDeclaration.value);
        value = std::make_shared<Node>(*value);
        value->Meta.is_const = false;
        if (value->type == NodeType::HOOK) {
            value->Hook.name = decl->VariableDeclaration.name;
        }
        Symbol symbol = new_symbol(decl->VariableDeclaration.name, value);
        add_symbol(symbol, current_symbol_table);
    }
    return new_boolean_node(true);
}

node_ptr Interpreter::eval_list(node_ptr node) {
    node_ptr list = new_node(NodeType::LIST);
    if (node->List.elements.size() == 1) {
        if (node->List.elements[0]->type == NodeType::COMMA_LIST) {
            for (auto elem : node->List.elements[0]->List.elements) {
                list->List.elements.push_back(eval_node(elem));
            }
        } else {
            list->List.elements.push_back(eval_node(node->List.elements[0]));
        }
    } else {
        list = node;
    }
    return list;
}

node_ptr Interpreter::eval_object(node_ptr node) {
    node_ptr object = new_node();
    object->type = NodeType::OBJECT;
    // inject current object into scope as "this"
    add_symbol(new_symbol("this", object), current_symbol_table);
    if (node->Object.elements[0]->type != NodeType::COMMA_LIST) {
        node_ptr prop = node->Object.elements[0];
        if (prop->Operator.value != ":") {
            error_and_exit("Object must contain properties separated with ':'");
        }
        if (prop->Operator.left->type != NodeType::ID && prop->Operator.left->type != NodeType::STRING) {
            error_and_exit("Property names must be identifiers or strings");
        }
        object->Object.properties[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = eval_node(prop->Operator.right);
    }
    for (node_ptr prop : node->Object.elements[0]->List.elements) {
        if (prop->Operator.value != ":") {
            error_and_exit("Object must contain properties separated with ':'");
        }
        if (prop->Operator.left->type != NodeType::ID && prop->Operator.left->type != NodeType::STRING) {
            error_and_exit("Property names must be identifiers or strings");
        }
        object->Object.properties[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = eval_node(prop->Operator.right);
    }
    // remove "this" from scope
    delete_symbol("this", current_symbol_table);
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
    if (node->FuncCall.name == "type") {
        if (node->FuncCall.args.size() != 1) {
            error_and_exit("Function " + node->FuncCall.name + " expects 1 argument");
        }

        node_ptr var = eval_node(node->FuncCall.args[0]);

        switch(var->type) {
            case NodeType::NONE: return new_string_node("None");
            case NodeType::NUMBER: return new_string_node("Number");
            case NodeType::STRING: return new_string_node("String");
            case NodeType::BOOLEAN: return new_string_node("Boolean");
            case NodeType::LIST: return new_string_node("List");
            case NodeType::OBJECT: return new_string_node("Object");
            case NodeType::FUNC: return new_string_node("Function");
            default: return new_string_node("None");
        }
    }
    if (node->FuncCall.name == "load_lib") {
        return eval_load_lib(node);
    }
    if (node->FuncCall.name == "exit") {
        if (node->FuncCall.args.size() != 1) {
            exit(1);
        }

        node_ptr status_code = eval_node(node->FuncCall.args[0]);

        if (status_code->type != NodeType::NUMBER) {
            exit(1);
        } else {
            exit(status_code->Number.value);
        }
    }

    if (func != nullptr) {
        function->Function.name = func->Function.name;
        function->Function.args = std::vector<node_ptr>(func->Function.args);
        function->Function.params = std::vector<node_ptr>(func->Function.params);
        function->Function.body = func->Function.body;
        function->Function.closure = func->Function.closure;
        function->Function.is_hook = func->Function.is_hook;
        function->Function.decl_filename = func->Function.decl_filename;
        function->Hooks.onCall = func->Hooks.onCall;
    } else if (node->FuncCall.caller == nullptr) {
        Symbol function_symbol = get_symbol(node->FuncCall.name, current_symbol_table);
        if (function_symbol.value == nullptr) {
            error_and_exit("Function '" + node->FuncCall.name + "' is undefined");
        }
        if (function_symbol.value->type != NodeType::FUNC) {
            error_and_exit("Variable '" + node->FuncCall.name + "' is not a function");
        }
        function->Function.name = function_symbol.name;
        function->Function.args = std::vector<node_ptr>(function_symbol.value->Function.args);
        function->Function.params = std::vector<node_ptr>(function_symbol.value->Function.params);
        function->Function.body = function_symbol.value->Function.body;
        function->Function.closure = function_symbol.value->Function.closure;
        function->Function.is_hook = function_symbol.value->Function.is_hook;
        function->Function.decl_filename = function_symbol.value->Function.decl_filename;
        function->Hooks.onCall = function_symbol.value->Hooks.onCall;
    } else {
        node_ptr method = node->FuncCall.caller->Object.properties[node->FuncCall.name];
        if (method->type == NodeType::NONE) {
            error_and_exit("Method '" + node->FuncCall.name + "' does not exist");
        }
        if (method->type != NodeType::FUNC) {
            error_and_exit("Variable '" + node->FuncCall.name + "' is not a function");
        }
        function->Function.name = method->Function.name;
        function->Function.args = std::vector<node_ptr>(method->Function.args);
        function->Function.params = std::vector<node_ptr>(method->Function.params);
        function->Function.body = method->Function.body;
        function->Function.closure = method->Function.closure;
        function->Function.is_hook = method->Function.is_hook;
        function->Function.decl_filename = method->Function.decl_filename;
        function->Hooks.onCall = method->Hooks.onCall;
    }

    std::vector<node_ptr> args;
    for (node_ptr arg : node->FuncCall.args) {
        args.push_back(eval_node(arg));
    }

    auto local_scope = std::make_shared<SymbolTable>();
    auto current_scope = std::make_shared<SymbolTable>();

    current_symbol_table->child = local_scope;
    local_scope->parent = current_symbol_table;

    local_scope->child = current_scope;
    current_scope->parent = local_scope;

    // current_symbol_table 
    //    -> local_scope 
    //        -> current_scope

    if (node->FuncCall.caller != nullptr) {
        current_scope->symbols["this"] = new_symbol("this", node->FuncCall.caller);
    }

    current_symbol_table = current_scope;
    current_symbol_table->filename = function->Function.name + ": " + function->Function.decl_filename;
    
    // Inject closure into local scope

    for (auto& elem : function->Function.closure) {
        add_symbol(new_symbol(elem.first, elem.second), local_scope);
    }

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
            add_symbol(symbol, current_symbol_table);
        }
    }

    for (int i = 0; i < args.size(); i++) {
        std::string name = function->Function.params[i+start_index]->ID.value;
        node_ptr value = args[i];
        Symbol symbol = new_symbol(name, value);
        add_symbol(symbol, current_symbol_table);
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
                    if (evaluated_expr->Return.value == nullptr) {
                        res = new_node(NodeType::NONE);
                    } else {
                        res = eval_node(evaluated_expr->Return.value);
                    }
                    break;
                } else if (i == function->Function.body->Object.elements.size()-1) {
                    res = evaluated_expr;
                    if (res->type == NodeType::RETURN) {
                        if (res->Return.value == nullptr) {
                            res = new_node(NodeType::NONE);
                        } else {
                            res = res->Return.value;
                        }
                    }
                    break;
                }
            }
        }
    }

    auto allOnCallFunctionsLists = {std::cref(function->Hooks.onCall), std::cref(global_symbol_table->globalHooks_onCall)};

    if (!function->Function.is_hook) {      
        for (const auto& function_list : allOnCallFunctionsLists) {
            for (node_ptr func : function_list.get()) {
                node_ptr function_call = new_node(NodeType::FUNC_CALL);
                function_call->FuncCall.name = func->Function.name;
                function_call->FuncCall.args = std::vector<node_ptr>();
                if (func->Function.params.size() > 0) {
                    node_ptr file_info = new_node(NodeType::OBJECT);
                    file_info->Object.properties["name"] = new_string_node(function->Function.name);
                    node_ptr args_list = new_node(NodeType::LIST);
                    for (node_ptr arg : args) {
                        args_list->List.elements.push_back(arg);
                    }
                    file_info->Object.properties["args"] = args_list;
                    file_info->Object.properties["result"] = res;
                    file_info->Object.properties["filename"] = new_string_node(file_name);
                    file_info->Object.properties["line"] = new_number_node(line);
                    file_info->Object.properties["column"] = new_number_node(column);
                    function_call->FuncCall.args.push_back(file_info);
                }
                eval_func_call(function_call, func);
            }
        }
    }

    current_symbol_table = current_symbol_table->parent->parent;
    current_symbol_table->child->child = nullptr;
    current_symbol_table->child = nullptr;

    if (res->type == NodeType::FUNC) {
        for (auto& elem : current_symbol_table->symbols)
        res->Function.closure[elem.first] = elem.second.value;
    }

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
            if (evaluated_expr->type == NodeType::BREAK) {
                return evaluated_expr;
            }
            if (evaluated_expr->type == NodeType::CONTINUE) {
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
                if (expr->type == NodeType::BREAK) {
                    return eval_node(expr);
                }
                if (expr->type == NodeType::CONTINUE) {
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
    if (ret->Return.value == nullptr) {
        ret->Return.value = new_node(NodeType::NONE);
    } else {
        ret->Return.value = eval_node(node->Return.value);
    }
    return ret;
}

node_ptr Interpreter::eval_for_loop(node_ptr node) {

    if (node->ForLoop.iterator != nullptr) {
        node_ptr iterator = eval_node(node->ForLoop.iterator);

        // TODO: Implement for object looping

        if (iterator->type != NodeType::LIST) {
            error_and_exit("For loop iterator must be a list");
        }

        auto current_scope = current_symbol_table;
        
        for (int i = 0; i < iterator->List.elements.size(); i++) {

            auto loop_symbol_tale = std::make_shared<SymbolTable>();
            for (auto& symbol : current_symbol_table->symbols) {
                loop_symbol_tale->symbols[symbol.first] = symbol.second;
            }

            current_symbol_table = loop_symbol_tale;

            if (node->ForLoop.index_name) {
                add_symbol(new_symbol(node->ForLoop.index_name->ID.value, new_number_node(i)), current_symbol_table);
            }
            if (node->ForLoop.value_name) {
                add_symbol(new_symbol(node->ForLoop.value_name->ID.value, iterator->List.elements[i]), current_symbol_table);
            }
            for (node_ptr expr : node->ForLoop.body->Object.elements) {
                node_ptr evaluated_expr = eval_node(expr);
                if (evaluated_expr->type == NodeType::RETURN) {
                    current_symbol_table = current_scope;
                    return evaluated_expr;
                }
                if (evaluated_expr->type == NodeType::BREAK) {
                    goto _break;
                }
                if (evaluated_expr->type == NodeType::CONTINUE) {
                    goto _continue;
                }
            }

            _continue:
                current_symbol_table = current_scope;
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

        current_symbol_table = current_scope;

        return new_node(NodeType::NONE);
    }

    node_ptr start_node = eval_node(node->ForLoop.start);
    node_ptr end_node = eval_node(node->ForLoop.end);

    if (start_node->type != NodeType::NUMBER && end_node->type != NodeType::NUMBER) {
        error_and_exit("For loop range expects two numbers");
    }

    int start = start_node->Number.value;
    int end = end_node->Number.value;

    auto current_scope = current_symbol_table;

    int for_index = -1;
    
    for (int i = start; i < end; i++) {

        for_index++;

        auto loop_symbol_tale = std::make_shared<SymbolTable>();
        for (auto& symbol : current_symbol_table->symbols) {
            loop_symbol_tale->symbols[symbol.first] = symbol.second;
        }

        current_symbol_table = loop_symbol_tale;

        int index = i-node->ForLoop.start->Number.value;
        if (node->ForLoop.index_name) {
            add_symbol(new_symbol(node->ForLoop.index_name->ID.value, new_number_node(for_index)), current_symbol_table);
        }
        if (node->ForLoop.value_name) {
        add_symbol(new_symbol(
            node->ForLoop.value_name->ID.value, new_number_node(i)), current_symbol_table);
        }
        for (node_ptr expr : node->ForLoop.body->Object.elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                current_symbol_table = current_scope;
                return evaluated_expr;
            }
            if (evaluated_expr->type == NodeType::BREAK) {
                goto _break2;
            }
            if (evaluated_expr->type == NodeType::CONTINUE) {
                goto _continue2;
            }
        }

        _continue2:
            current_symbol_table = current_scope;
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

    current_symbol_table = current_scope;

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_while_loop(node_ptr node) {
    node_ptr conditional = eval_node(node->WhileLoop.condition);
    if (conditional->type != NodeType::BOOLEAN) {
        error_and_exit("While loop conditional must evaluate to a boolean");
    }

    auto current_scope = current_symbol_table;

    while (conditional->Boolean.value) {
        auto loop_symbol_tale = std::make_shared<SymbolTable>();
        for (auto& symbol : current_symbol_table->symbols) {
            loop_symbol_tale->symbols[symbol.first] = symbol.second;
        }

        current_symbol_table = loop_symbol_tale;
        
        for (node_ptr expr : node->WhileLoop.body->Object.elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                current_symbol_table = current_scope;
                return evaluated_expr->Return.value;
            }
            if (evaluated_expr->type == NodeType::BREAK) {
                goto _break;
            }
        }

        conditional = eval_node(node->WhileLoop.condition);

        current_symbol_table = current_scope;
    }

    _break:
        current_symbol_table = current_scope;

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

node_ptr Interpreter::eval_function(node_ptr node) {
    node->Function.decl_filename = file_name;
    // Inject current scope as closure
    for (auto& symbol : current_symbol_table->symbols) {
        node->Function.closure[symbol.first] = symbol.second.value;
    }
    return node;
}

node_ptr Interpreter::eval_type(node_ptr node) {
    node_ptr object = new_node(NodeType::OBJECT);

    if (node->Type.body->Object.elements[0]->type != NodeType::COMMA_LIST) {
        node_ptr prop = node->Type.body->Object.elements[0];

        if (prop->Operator.value != ":") {
            error_and_exit("Object must contain properties separated with ':'");
        }
        if (prop->Operator.left->type != NodeType::ID && prop->Operator.left->type != NodeType::STRING) {
            error_and_exit("Property names must be identifiers or strings");
        }

        node_ptr value = prop->Operator.right;
        if (value->type == NodeType::ACCESSOR) {
            object->Object.properties[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = eval_node(value->Accessor.container);
            node_ptr default_value = eval_node(value->Accessor.accessor);
            if (default_value->List.elements.size() != 1) {
                error_and_exit("Default type constructor cannot be empty");
            }
            object->Object.properties[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = eval_node(value->Accessor.container);
            object->Object.defaults[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = default_value->List.elements[0];
        } else {
            object->Object.properties[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = eval_node(prop->Operator.right);
        }
    } else {
        for (node_ptr prop : node->Type.body->Object.elements[0]->List.elements) {

            if (prop->Operator.value != ":") {
                error_and_exit("Object must contain properties separated with ':'");
            }
            if (prop->Operator.left->type != NodeType::ID && prop->Operator.left->type != NodeType::STRING) {
                error_and_exit("Property names must be identifiers or strings");
            }
            
            node_ptr value = prop->Operator.right;
            if (value->type == NodeType::ACCESSOR) {
                object->Object.properties[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = eval_node(value->Accessor.container);
                node_ptr default_value = eval_node(value->Accessor.accessor);
                if (default_value->List.elements.size() != 1) {
                    error_and_exit("Default type constructor cannot be empty");
                }
                object->Object.properties[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = eval_node(value->Accessor.container);
                object->Object.defaults[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = default_value->List.elements[0];
            } else {
                object->Object.properties[prop->Operator.left->type == NodeType::ID ? prop->Operator.left->ID.value: prop->Operator.left->String.value] = eval_node(prop->Operator.right);
            }
        }
    }

    object->Object.is_type = true;
    object->TypeInfo.type_name = node->Type.name;
    Symbol symbol = new_symbol(node->Type.name, object);
    add_symbol(symbol, current_symbol_table);
    return new_node(NodeType::NONE);
}

bool Interpreter::match_types(node_ptr nodeA, node_ptr nodeB) {
    if (nodeA == nullptr || nodeB == nullptr) {
        return false;
    }

    if (nodeA->type != nodeB->type) {
        return false;
    }

    if (nodeA->type == NodeType::OBJECT) {
        if (nodeA->TypeInfo.type_name != nodeB->TypeInfo.type_name) {
            return false;
        }
    }

    return true;
}

node_ptr Interpreter::eval_object_init(node_ptr node) {
    Symbol type_symbol = get_symbol(node->ObjectDeconstruct.name, current_symbol_table);
    if (type_symbol.value == nullptr) {
        error_and_exit("Type '" + node->ObjectDeconstruct.name + "' is undefined");
    }
    node_ptr type = type_symbol.value;

    if (!type->Object.is_type) {
        error_and_exit("Variable '" + node->ObjectDeconstruct.name + "' is not a type");
    }
    
    node_ptr object = new_node(NodeType::OBJECT);
    node_ptr init_props = eval_object(node->ObjectDeconstruct.body);
    object->TypeInfo.type = type_symbol.value;

    // Add defaults if they exist

    for (auto def : type->Object.defaults) {
        object->Object.properties[def.first] = def.second;
    }

    // Add Init props

    for (auto prop : init_props->Object.properties) {
        object->Object.properties[prop.first] = prop.second;
    }

    // Check that the value matches type
    // TODO: comprehensive type check goes here
    if (object->Object.properties.size() != type->Object.properties.size()) {
        error_and_exit("Error in object initialization for type '" + node->ObjectDeconstruct.name + "': Number of properties differs between object and type");
    }

    for (auto& prop : type->Object.properties) {
        std::string prop_name = prop.first;

        node_ptr type_prop_value = prop.second;
        node_ptr obj_prop_value = object->Object.properties[prop_name];

        int match = match_types(obj_prop_value, type_prop_value);
        if (!match) {
            error_and_exit("Error in object initialization for type '" + node->ObjectDeconstruct.name + "': Match error in property '" + prop_name + "'");
        }
    }

    object->TypeInfo.type = type;
    object->TypeInfo.type_name = node->ObjectDeconstruct.name;
    return object;
}

node_ptr Interpreter::eval_load_lib(node_ptr node) {
    auto args = node->FuncCall.args;
    if (args.size() != 1) {
        error_and_exit("Library loading expects 1 argument");
    }

    node_ptr path = eval_node(args[0]);

    if (path->type != NodeType::STRING) {
        error_and_exit("Library loading expects 1 string argument");
    }

    node_ptr lib_node = new_node(NodeType::LIB);

    void* handle = dlopen(path->String.value.c_str(), RTLD_LAZY);
    
    if (!handle) {
        error_and_exit("Cannot open library: " + std::string(dlerror()));
    }

    typedef void* (*load_t)();
    typedef node_ptr (*call_function_t)(std::string name, void* handle, std::vector<node_ptr> args);

    dlerror();

    load_t load = (load_t) dlsym(handle, "load");
    const char* dlsym_error_load = dlerror();
    call_function_t call_function = (call_function_t) dlsym(handle, "call_function");
    const char* dlsym_error_call_function = dlerror();

    if (dlsym_error_load) {
        dlclose(handle);
        error_and_exit("Error loading symbol 'load': " + std::string(dlsym_error_load));
    }

    if (dlsym_error_call_function) {
        dlclose(handle);
        error_and_exit("Error loading symbol 'call_function': " + std::string(dlsym_error_call_function));
    }

    lib_node->Library.handle = load();
    lib_node->Library.call_function = call_function;

    return lib_node;
}

node_ptr Interpreter::eval_call_lib_function(node_ptr lib, node_ptr& node) {
    auto args = node->FuncCall.args;
    if (args.size() != 2) {
        error_and_exit("Library function calls expects 2 arguments");
    }

    node_ptr name = eval_node(args[0]);
    node_ptr func_args = eval_node(args[1]);

    if (name->type != NodeType::STRING) {
        error_and_exit("Library function calls expects first argument to be a string");
    }

    if (func_args->type != NodeType::LIST) {
        error_and_exit("Library function calls expects first argument to be a list");
    }

    return lib->Library.call_function(name->String.value, lib->Library.handle, func_args->List.elements);
}

node_ptr Interpreter::eval_import(node_ptr node) {
    if (node->Import.target->type != NodeType::STRING) {
        error_and_exit("Import target must be a string");
    }

    std::string path = node->Import.target->String.value + ".vtx";

    if (node->Import.module->type == NodeType::ID) {
        std::string module_name = node->Import.module->ID.value;
        node_ptr import_obj = new_node(NodeType::OBJECT);

        Lexer import_lexer(path);
        import_lexer.file_name = path;
        import_lexer.tokenize();

        Parser import_parser(import_lexer.nodes, import_lexer.file_name);
        import_parser.parse(0, "_");
        import_parser.remove_op_node(";");
        
        auto current_path = std::filesystem::current_path();
        auto parent_path = std::filesystem::path(path).parent_path();
        try {
            std::filesystem::current_path(parent_path);
        } catch(...) {
            error_and_exit("No such file or directory: '" + parent_path.string() + "'");
        }

        Interpreter import_interpreter(import_parser.nodes, import_parser.file_name);
        import_interpreter.evaluate();

        std::filesystem::current_path(current_path);

        for (auto& symbol : import_interpreter.global_symbol_table->symbols) {
            import_obj->Object.properties[symbol.first] = symbol.second.value;
        }

        // We also want to import hooks into the current scope

        for (auto& hook : import_interpreter.global_symbol_table->globalHooks_onChange) {
            current_symbol_table->globalHooks_onChange.push_back(hook);
        }

        for (auto& hook : import_interpreter.global_symbol_table->globalHooks_onCall) {
            current_symbol_table->globalHooks_onCall.push_back(hook);
        }

        add_symbol(new_symbol(module_name, import_obj), current_symbol_table);
        return new_node(NodeType::NONE);
    }

    if (node->Import.module->type == NodeType::LIST) {
        // Before we import, we'll check to see if the import list
        // contains only IDs
        if (node->Import.module->List.elements.size() == 1 && node->Import.module->List.elements[0]->type == NodeType::COMMA_LIST) {
            node->Import.module = node->Import.module->List.elements[0];
        }
        for (node_ptr elem : node->Import.module->List.elements) {
            if (elem->type != NodeType::ID) {
                error_and_exit("Import list must contain identifiers");
            }
        }

        Lexer import_lexer(path);
        import_lexer.file_name = path;
        import_lexer.tokenize();

        Parser import_parser(import_lexer.nodes, import_lexer.file_name);
        import_parser.parse(0, "_");
        import_parser.remove_op_node(";");

        auto current_path = std::filesystem::current_path();
        auto parent_path = std::filesystem::path(path).parent_path();
        try {
            std::filesystem::current_path(parent_path);
        } catch(...) {
            error_and_exit("No such file or directory: '" + parent_path.string() + "'");
        }

        Interpreter import_interpreter(import_parser.nodes, import_parser.file_name);
        import_interpreter.evaluate();

        std::filesystem::current_path(current_path);

        std::unordered_map<std::string, node_ptr> imported_variables;

        for (auto& symbol : import_interpreter.global_symbol_table->symbols) {
            imported_variables[symbol.first] = symbol.second.value;
        }

        for (node_ptr elem : node->Import.module->List.elements) {
            if (imported_variables.contains(elem->ID.value)) {
                add_symbol(import_interpreter.global_symbol_table->symbols[elem->ID.value], current_symbol_table);
            } else {
                error_and_exit("Cannot import value '" + elem->ID.value + "' - variable undefined");
            }
        }

        return new_node(NodeType::NONE);
    }
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

node_ptr Interpreter::eval_not(node_ptr node) {
    node_ptr value = eval_node(node->Operator.right);
    if (value->type != NodeType::BOOLEAN) {
        error_and_exit("Cannot negate a non-boolean");
    }

    return new_boolean_node(!value->Boolean.value);
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

    if (left->type == NodeType::LIST && right->type == NodeType::LIST) {
        node_ptr result = new_node(NodeType::LIST);
        result->List.elements.insert(result->List.elements.end(), left->List.elements.begin(), left->List.elements.end());
        result->List.elements.insert(result->List.elements.end(), right->List.elements.begin(), right->List.elements.end());
        return result;
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

    if (left->type == NodeType::NONE) {
        return new_boolean_node(true);
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

    if (left->type == NodeType::NONE) {
        return new_boolean_node(false);
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

node_ptr Interpreter::eval_plus_eq(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    node_ptr plus_node = new_node(NodeType::OP);
    plus_node->Operator.value = "+";
    plus_node->Operator.left = left;
    plus_node->Operator.right = right;
    plus_node = eval_add(plus_node);

    node_ptr eq_node = new_node(NodeType::OP);
    eq_node->Operator.value = "=";
    eq_node->Operator.left = node->Operator.left;
    eq_node->Operator.right = plus_node;
    eq_node = eval_eq(eq_node);

    return eq_node;
}

node_ptr Interpreter::eval_minus_eq(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = eval_node(node->Operator.right);

    node_ptr minus_node = new_node(NodeType::OP);
    minus_node->Operator.value = "-";
    minus_node->Operator.left = left;
    minus_node->Operator.right = right;
    minus_node = eval_sub(minus_node);

    node_ptr eq_node = new_node(NodeType::OP);
    eq_node->Operator.value = "=";
    eq_node->Operator.left = node->Operator.left;
    eq_node->Operator.right = minus_node;
    eq_node = eval_eq(eq_node);

    return eq_node;
}

node_ptr Interpreter::eval_dot(node_ptr node) {
    node_ptr left = eval_node(node->Operator.left);
    node_ptr right = node->Operator.right;

    if (left->type == NodeType::OBJECT) {

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

        if (right->type == NodeType::ACCESSOR) {
            node_ptr left_side = new_node(NodeType::OP);
            left_side->Operator.value = ".";
            left_side->Operator.left = left;
            if (right->Accessor.container->type != NodeType::ID) {
                error_and_exit("Malformed '.' operation");
            }
            left_side->Operator.right = right->Accessor.container;
            left_side = eval_dot(left_side);

            node_ptr res = new_node(NodeType::ACCESSOR);
            res->Accessor.container = left_side;
            res->Accessor.accessor = right->Accessor.accessor;
            return eval_accessor(res);
        }

        error_and_exit("Right hand side of '.' must be an identifier, function call or accessor");
    }

    if (left->type == NodeType::LIST) {
        
        // List Properties
        if (right->type == NodeType::ID) {
            std::string prop = right->ID.value;

            if (prop == "length") {
                return new_number_node(left->List.elements.size());
            }
            if (prop == "empty") {
                return new_boolean_node(left->List.elements.empty());
            }

            error_and_exit("List does not have property '" + prop + "'");
        }

        if (right->type == NodeType::FUNC_CALL) {
            std::string prop = right->FuncCall.name;

            if (prop == "append") {
                if (left->Meta.is_const) {
                    error_and_exit("Cannot modify constant list");
                }
                if (right->FuncCall.args.size() != 1) {
                    error_and_exit("List function '" + prop + "' expects 1 argument");
                }
                node_ptr arg = eval_node(right->FuncCall.args[0]);
                left->List.elements.push_back(arg);
                return left;
            }
            if (prop == "prepend") {
                if (left->Meta.is_const) {
                    error_and_exit("Cannot modify constant list");
                }
                if (right->FuncCall.args.size() != 1) {
                    error_and_exit("List function '" + prop + "' expects 1 argument");
                }
                node_ptr arg = right->FuncCall.args[0];
                left->List.elements.insert(left->List.elements.begin(), eval_node(arg));
                return left;
            }
            if (prop == "insert") {
                if (left->Meta.is_const) {
                    error_and_exit("Cannot modify constant list");
                }
                if (right->FuncCall.args.size() != 2) {
                    error_and_exit("List function '" + prop + "' expects 2 arguments");
                }
                node_ptr value = eval_node(right->FuncCall.args[0]);
                node_ptr index_node = eval_node(right->FuncCall.args[1]);

                if (index_node->type != NodeType::NUMBER) {
                    error_and_exit("List function '" + prop + "' expects second argument to be a number");
                }

                int index = index_node->Number.value;

                if (index < 0) {
                    index = 0;
                } else if (index > left->List.elements.size()) {
                    index = left->List.elements.size();
                }

                left->List.elements.insert(left->List.elements.begin() + index, value);
                return left;
            }
            if (prop == "remove_at") {
                if (left->Meta.is_const) {
                    error_and_exit("Cannot modify constant list");
                }
                if (left->List.elements.size() == 0) {
                    return left;
                }

                if (right->FuncCall.args.size() != 1) {
                    error_and_exit("List function '" + prop + "' expects 1 argument");
                }

                node_ptr index_node = eval_node(right->FuncCall.args[0]);

                if (index_node->type != NodeType::NUMBER) {
                    error_and_exit("List function '" + prop + "' expects argument to be a number");
                }

                int index = index_node->Number.value;

                if (index < 0) {
                    index = 0;
                } else if (index > left->List.elements.size()-1) {
                    index = left->List.elements.size()-1;
                }

                left->List.elements.erase(left->List.elements.begin() + index);
                return left;
            }
            if (prop == "remove") {
                if (left->Meta.is_const) {
                    error_and_exit("Cannot modify constant list");
                }
                if (right->FuncCall.args.size() != 1) {
                    error_and_exit("List function '" + prop + "' expects 1 argument");
                }
                node_ptr value = eval_node(right->FuncCall.args[0]);

                for (int _index = 0; _index < left->List.elements.size(); _index++) {
                    node_ptr eq_eq = new_node(NodeType::OP);
                    eq_eq->Operator.value = "==";
                    eq_eq->Operator.left = left->List.elements[_index];
                    eq_eq->Operator.right = value;
                    eq_eq = eval_eq_eq(eq_eq);
                    if (eq_eq->Boolean.value) {
                        left->List.elements.erase(left->List.elements.begin() + _index);
                        _index--;
                    }
                }

                return left;
            }
            if (prop == "remove_if") {
                if (left->Meta.is_const) {
                    error_and_exit("Cannot modify constant list");
                }
                if (right->FuncCall.args.size() != 1) {
                    error_and_exit("List function '" + prop + "' expects 1 argument");
                }
                
                node_ptr function = right->FuncCall.args[0];

                if (function->type != NodeType::FUNC) {
                    error_and_exit("List function '" + prop + "' expects 1 function argument");
                }

                for (int _index = 0; _index < left->List.elements.size(); _index++) {
                    node_ptr value = left->List.elements[_index];
                    node_ptr func_call = new_node(NodeType::FUNC_CALL);
                    if (function->Function.params.size() == 0) {
                        error_and_exit("Function needs to have at least one parameter");
                    }
                    if (function->Function.params.size() > 0) {
                        func_call->FuncCall.args.push_back(value);
                    }
                    if (function->Function.params.size() > 1) {
                        func_call->FuncCall.args.push_back(new_number_node(_index));
                    }
                    if (function->Function.params.size() > 2) {
                        func_call->FuncCall.args.push_back(left);
                    }
                    node_ptr res = eval_func_call(func_call, function);
                    if (res->type != NodeType::BOOLEAN) {
                        error_and_exit("Function must return a boolean value");
                    }
                    if (res->Boolean.value) {
                        left->List.elements.erase(left->List.elements.begin() + _index);
                        _index--;
                    }
                }

                return left;
            }

            // Functional Operations

            if (prop == "map") {
                if (right->FuncCall.args.size() != 1) {
                    error_and_exit("List function '" + prop + "' expects 1 argument");
                }
                
                node_ptr function = eval_node(right->FuncCall.args[0]);

                if (function->type != NodeType::FUNC) {
                    error_and_exit("List function '" + prop + "' expects 1 function argument");
                }

                node_ptr new_list = new_node(NodeType::LIST);

                for (int _index = 0; _index < left->List.elements.size(); _index++) {
                    node_ptr value = left->List.elements[_index];
                    node_ptr func_call = new_node(NodeType::FUNC_CALL);
                    if (function->Function.params.size() == 0) {
                        error_and_exit("Function needs to have at least one parameter");
                    }
                    if (function->Function.params.size() > 0) {
                        func_call->FuncCall.args.push_back(value);
                    }
                    if (function->Function.params.size() > 1) {
                        func_call->FuncCall.args.push_back(new_number_node(_index));
                    }
                    if (function->Function.params.size() > 2) {
                        func_call->FuncCall.args.push_back(left);
                    }
                    node_ptr res = eval_func_call(func_call, function);

                    new_list->List.elements.push_back(res);
                }

                return new_list;
            }

            if (prop == "filter") {
                if (right->FuncCall.args.size() != 1) {
                    error_and_exit("List function '" + prop + "' expects 1 argument");
                }
                
                node_ptr function = eval_node(right->FuncCall.args[0]);

                if (function->type != NodeType::FUNC) {
                    error_and_exit("List function '" + prop + "' expects 1 function argument");
                }

                node_ptr new_list = new_node(NodeType::LIST);

                for (int _index = 0; _index < left->List.elements.size(); _index++) {
                    node_ptr value = left->List.elements[_index];
                    node_ptr func_call = new_node(NodeType::FUNC_CALL);
                    if (function->Function.params.size() == 0) {
                        error_and_exit("Function needs to have at least one parameter");
                    }
                    if (function->Function.params.size() > 0) {
                        func_call->FuncCall.args.push_back(value);
                    }
                    if (function->Function.params.size() > 1) {
                        func_call->FuncCall.args.push_back(new_number_node(_index));
                    }
                    if (function->Function.params.size() > 2) {
                        func_call->FuncCall.args.push_back(left);
                    }
                    node_ptr res = eval_func_call(func_call, function);

                    if (res->type != NodeType::BOOLEAN) {
                        error_and_exit("Function must return a boolean value");
                    }

                    if (res->Boolean.value) {
                        new_list->List.elements.push_back(value);
                    }
                }

                return new_list;
            }

            if (prop == "reduce") {
                if (right->FuncCall.args.size() != 1) {
                    error_and_exit("List function '" + prop + "' expects 1 argument");
                }
                
                node_ptr function = eval_node(right->FuncCall.args[0]);

                if (function->type != NodeType::FUNC) {
                    error_and_exit("List function '" + prop + "' expects 1 function argument");
                }

                node_ptr new_list = new_node(NodeType::LIST);

                if (left->List.elements.size() < 2) {
                    return left;
                }

                for (int _index = 0; _index < left->List.elements.size()-1; _index++) {
                    node_ptr func_call = new_node(NodeType::FUNC_CALL);
                    if (function->Function.params.size() < 2) {
                        error_and_exit("Function needs to have at least two parameters");
                    }

                    node_ptr value = left->List.elements[_index];
                    node_ptr next_value = left->List.elements[_index + 1];

                    if (new_list->List.elements.size() > 0) {
                        value = new_list->List.elements[0];
                    }

                    func_call->FuncCall.args.push_back(value);
                    func_call->FuncCall.args.push_back(next_value);

                    if (function->Function.params.size() > 2) {
                        func_call->FuncCall.args.push_back(new_number_node(_index));
                    }
                    if (function->Function.params.size() > 3) {
                        func_call->FuncCall.args.push_back(left);
                    }

                    node_ptr res = eval_func_call(func_call, function);
                    value = res;

                    if (new_list->List.elements.size() == 0) {
                        new_list->List.elements.push_back(res);
                    } else {
                        new_list->List.elements[0] = res;
                    }
                }

                return new_list->List.elements[0];
            }
        }

        error_and_exit("Right hand side of '.' must be an identifier or function call");
    }

    if (left->type == NodeType::HOOK) {
        std::string hook_name = left->Hook.hook_name;
        std::string name = left->Hook.name;
        node_ptr hook_func = eval_node(left->Hook.function);
        hook_func->Function.name = name;
        hook_func->Function.is_hook = true;

        if (right->type == NodeType::FUNC_CALL) {
            std::string func_name = right->FuncCall.name;

            // TODO: Change hooks to be map
            // Leaving as list due to current implementations

            if (func_name == "attach") {
                if (right->FuncCall.args.size() == 0) {
                    if (hook_name == "onChange") {
                        global_symbol_table->globalHooks_onChange.push_back(hook_func);
                    }
                    if (hook_name == "onCall") {
                        global_symbol_table->globalHooks_onCall.push_back(hook_func);
                    }
                    return new_node(NodeType::NONE);
                }
                if (right->FuncCall.args.size() == 1) {
                    node_ptr arg = eval_node(right->FuncCall.args[0]);

                    if (hook_name == "onChange") {
                        if (arg->type == NodeType::LIST) {
                            for (auto elem : arg->List.elements) {
                                elem->Hooks.onChange.push_back(hook_func);
                            }
                        } else {
                            arg->Hooks.onChange.push_back(hook_func);
                        }
                    }
                    if (hook_name == "onCall") {
                        if (arg->type == NodeType::LIST) {
                            for (auto elem : arg->List.elements) {
                                elem->Hooks.onCall.push_back(hook_func);
                            }
                        } else {
                            arg->Hooks.onCall.push_back(hook_func);
                        }
                    }
                    return new_node(NodeType::NONE);
                }
            }
            if (func_name == "detach") {
                if (right->FuncCall.args.size() == 0) {
                    if (hook_name == "onChange") {
                        for (int i = 0; i < global_symbol_table->globalHooks_onChange.size(); i++) {
                            node_ptr func = global_symbol_table->globalHooks_onChange[i];
                            if (func->Function.name == name) {
                                global_symbol_table->globalHooks_onChange.erase(global_symbol_table->globalHooks_onChange.begin() + i);
                            }
                        }
                    }
                    if (hook_name == "onCall") {
                        for (int i = 0; i < global_symbol_table->globalHooks_onCall.size(); i++) {
                            node_ptr func = global_symbol_table->globalHooks_onCall[i];
                            if (func->Function.name == name) {
                                global_symbol_table->globalHooks_onCall.erase(global_symbol_table->globalHooks_onCall.begin() + i);
                            }
                        }
                    }
                    return new_node(NodeType::NONE);
                }
                if (right->FuncCall.args.size() == 1) {
                    node_ptr arg = eval_node(right->FuncCall.args[0]);

                    if (hook_name == "onChange") {
                        if (arg->type == NodeType::LIST) {
                            for (auto elem : arg->List.elements) {
                                for (int i = 0; i < elem->Hooks.onChange.size(); i++) {
                                    if (elem->Hooks.onChange[i]->Function.name == name) {
                                        elem->Hooks.onChange.erase(elem->Hooks.onChange.begin() + i);
                                    }
                                }
                            }
                        } else {
                            for (int i = 0; i < arg->Hooks.onChange.size(); i++) {
                                if (arg->Hooks.onChange[i]->Function.name == name) {
                                    arg->Hooks.onChange.erase(arg->Hooks.onChange.begin() + i);
                                }
                            }
                        }
                    }
                    if (hook_name == "onCall") {
                        if (arg->type == NodeType::LIST) {
                            for (auto elem : arg->List.elements) {
                                for (int i = 0; i < elem->Hooks.onCall.size(); i++) {
                                    if (elem->Hooks.onCall[i]->Function.name == name) {
                                        elem->Hooks.onCall.erase(elem->Hooks.onCall.begin() + i);
                                    }
                                }
                            }
                        } else {
                            for (int i = 0; i < arg->Hooks.onCall.size(); i++) {
                                if (arg->Hooks.onCall[i]->Function.name == name) {
                                    arg->Hooks.onCall.erase(arg->Hooks.onCall.begin() + i);
                                }
                            }
                        }
                    }
                    return new_node(NodeType::NONE);
                }
            }
        }
    }

    if (left->type == NodeType::LIB) {
        if (right->type == NodeType::FUNC_CALL) {
            if (right->FuncCall.name == "call") {
                return eval_call_lib_function(left, right);
            }
        }

        error_and_exit("Cannot perform '.' on types: " + node_repr(left) + ", " + node_repr(right));
    }
}

node_ptr Interpreter::eval_eq(node_ptr node) {
    node_ptr left = node->Operator.left;
    node_ptr right = eval_node(node->Operator.right);

    if (left->Operator.value == ".") {
        node_ptr object = eval_node(left->Operator.left);
        if (object->Meta.is_const) {
            error_and_exit("Cannot modify constant object");
        }
        node_ptr prop = left->Operator.right;

        if (object->type != NodeType::OBJECT) {
            error_and_exit("Left hand side of '.' must be an object");
        }

        if (prop->type != NodeType::ID && prop->type != NodeType::ACCESSOR) {
            error_and_exit("Right hand side of '.' must be an identifier");
        }

        if (prop->type == NodeType::ACCESSOR) {
            node_ptr accessed_value = eval_dot(left);
            node_ptr old_value = std::make_shared<Node>(*accessed_value);
            std::vector<node_ptr> onChangeFunctions = accessed_value->Hooks.onChange;
            *accessed_value = *right;
            accessed_value->Meta.is_const = false;
            accessed_value->Hooks.onChange = onChangeFunctions;

            auto allOnChangeFunctionsLists = {std::cref(onChangeFunctions), std::cref(global_symbol_table->globalHooks_onChange)};
            
            for (const auto& function_list : allOnChangeFunctionsLists) {
                for (auto function : function_list.get()) {
                    node_ptr function_call = new_node(NodeType::FUNC_CALL);
                    function_call->FuncCall.name = function->Function.name;
                    function_call->FuncCall.args = std::vector<node_ptr>();
                    if (function->Function.params.size() > 0) {
                        function_call->FuncCall.args.push_back(accessed_value);
                    }
                    if (function->Function.params.size() > 1) {
                        function_call->FuncCall.args.push_back(old_value);
                    }
                    if (function->Function.params.size() > 2) {
                        node_ptr file_info = new_node(NodeType::OBJECT);
                        file_info->Object.properties["name"] = new_string_node(printable(left->Operator.left) + "." + printable(prop));
                        file_info->Object.properties["filename"] = new_string_node(file_name);
                        file_info->Object.properties["line"] = new_number_node(line);
                        file_info->Object.properties["column"] = new_number_node(column);
                        function_call->FuncCall.args.push_back(file_info);
                    }
                    eval_func_call(function_call, function);
                }
            }

            return object;
        }

        node_ptr accessed_value = object->Object.properties[prop->ID.value];
        node_ptr old_value = std::make_shared<Node>(*accessed_value);
        std::vector<node_ptr> onChangeFunctions = old_value->Hooks.onChange;

        *accessed_value = *right;
        accessed_value->Hooks.onChange = onChangeFunctions;

        auto allOnChangeFunctionsLists = {std::cref(onChangeFunctions), std::cref(global_symbol_table->globalHooks_onChange)};
            
        for (const auto& function_list : allOnChangeFunctionsLists) {
            for (node_ptr function : function_list.get()) {
                node_ptr function_call = new_node(NodeType::FUNC_CALL);
                function_call->FuncCall.name = function->Function.name;
                function_call->FuncCall.args = std::vector<node_ptr>();
                if (function->Function.params.size() > 0) {
                    function_call->FuncCall.args.push_back(accessed_value);
                }
                if (function->Function.params.size() > 1) {
                    function_call->FuncCall.args.push_back(old_value);
                }
                if (function->Function.params.size() > 2) {
                    node_ptr file_info = new_node(NodeType::OBJECT);
                    file_info->Object.properties["name"] = new_string_node(printable(left->Operator.left) + "." + printable(prop));
                    file_info->Object.properties["filename"] = new_string_node(file_name);
                    file_info->Object.properties["line"] = new_number_node(line);
                    file_info->Object.properties["column"] = new_number_node(column);
                    function_call->FuncCall.args.push_back(file_info);
                }
                eval_func_call(function_call, function);
            }
        }

        return object;
    }

    if (left->type == NodeType::ACCESSOR) {
        node_ptr container = eval_node(left->Accessor.container);
        node_ptr accessor = eval_node(left->Accessor.accessor);
        if (container->Meta.is_const) {
            error_and_exit("Cannot modify constant");
        }
        if (container->type == NodeType::LIST) {
            accessor = eval_node(accessor->List.elements[0]);
            if (accessor->type != NodeType::NUMBER) {
                error_and_exit("List accessor must be a number");
            }

            node_ptr accessed_value = eval_accessor(left);

            if (accessor->Number.value < 0) {
                container->List.elements.insert(container->List.elements.begin(), right);
            } else if (accessor->Number.value >= container->List.elements.size()) {
                container->List.elements.push_back(right);
            } else {
                // if (accessed_value->Meta.is_const) {
                //     error_and_exit("Cannot modify constant");
                // }
                *accessed_value = *right;
                accessed_value->Meta.is_const = false;
            }
        } else if (container->type == NodeType::OBJECT) {
            node_ptr accessed_value = eval_accessor(left);

            if (accessed_value->type == NodeType::NONE) {
                container->Object.properties[accessor->List.elements[0]->String.value] = right;
            } else {
                node_ptr old_value = std::make_shared<Node>(*accessed_value);
                std::vector<node_ptr> onChangeFunctions = accessed_value->Hooks.onChange;
                *accessed_value = *right;
                accessed_value->Meta.is_const = false;
                accessed_value->Hooks.onChange = onChangeFunctions;

                auto allOnChangeFunctionsLists = {std::cref(onChangeFunctions), std::cref(global_symbol_table->globalHooks_onChange)};
                
                for (const auto& function_list : allOnChangeFunctionsLists) {
                    for (auto function : function_list.get()) {
                        node_ptr function_call = new_node(NodeType::FUNC_CALL);
                        function_call->FuncCall.name = function->Function.name;
                        function_call->FuncCall.args = std::vector<node_ptr>();
                        if (function->Function.params.size() > 0) {
                            function_call->FuncCall.args.push_back(accessed_value);
                        }
                        if (function->Function.params.size() > 1) {
                            function_call->FuncCall.args.push_back(old_value);
                        }
                        if (function->Function.params.size() > 2) {
                            node_ptr file_info = new_node(NodeType::OBJECT);
                            file_info->Object.properties["name"] = new_string_node(printable(left));
                            file_info->Object.properties["filename"] = new_string_node(file_name);
                            file_info->Object.properties["line"] = new_number_node(line);
                            file_info->Object.properties["column"] = new_number_node(column);
                            function_call->FuncCall.args.push_back(file_info);
                        }
                        eval_func_call(function_call, function);
                    }
                }
            }
        }
        return right;
    }

    if (left->type == NodeType::ID) {
        Symbol symbol = get_symbol(left->ID.value, current_symbol_table);
        if (symbol.value == nullptr) {
            error_and_exit("Variable '" + left->ID.value + "' is undefined");
        } else {
            // Re-assigning, check if const
            if (symbol.value->Meta.is_const) {
                error_and_exit("Cannot modify constant '" + symbol.name + "'");
            }

            node_ptr old_value = std::make_shared<Node>(*symbol.value);

            // Extract onChange functions from value

            std::vector<node_ptr> onChangeFunctions = symbol.value->Hooks.onChange;

            *symbol.value = *right;
            symbol.value->Meta.is_const = false;
            symbol.value->Hooks.onChange = onChangeFunctions;
            
            // Call onChange functions

            auto allOnChangeFunctionsLists = {std::cref(onChangeFunctions), std::cref(global_symbol_table->globalHooks_onChange)};
            
            for (const auto& function_list : allOnChangeFunctionsLists) {
                for (node_ptr function : function_list.get()) {
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
                        node_ptr file_info = new_node(NodeType::OBJECT);
                        file_info->Object.properties["name"] = new_string_node(symbol.name);
                        file_info->Object.properties["filename"] = new_string_node(file_name);
                        file_info->Object.properties["line"] = new_number_node(line);
                        file_info->Object.properties["column"] = new_number_node(column);
                        function_call->FuncCall.args.push_back(file_info);
                    }
                    eval_func_call(function_call, function);
                }
            }

            return right;
        }

        error_and_exit("Variable '" + left->ID.value + "' is undefined");
    }

    if (left->Operator.value == "::") {
        node_ptr target = left->Operator.left;
        node_ptr prop = left->Operator.right;

        right->Function.is_hook = true;

        if (prop->type != NodeType::ID) {
            error_and_exit("Hook expects an identifier");
        }

        if (target->type == NodeType::ID) {
            Symbol symbol = get_symbol(target->ID.value, current_symbol_table);
            if (symbol.value == nullptr) {
                error_and_exit("Variable '" + target->ID.value + "' is undefined");
            }

            if (prop->ID.value == "onChange") {
                if (right->type != NodeType::FUNC) {
                    error_and_exit("onChange hook expects a function");
                }
                symbol.value->Hooks.onChange.push_back(right);
            } else if (prop->ID.value == "onCall") {
                if (right->type != NodeType::FUNC && symbol.value->type != NodeType::FUNC) {
                    error_and_exit("onCall hook expects a function");
                }
                symbol.value->Hooks.onCall.push_back(right);
            } else {
                error_and_exit("Unknown hook '" + prop->ID.value + "'");
            }

            add_symbol(symbol, current_symbol_table);

            return new_node(NodeType::NONE);
        }

        if (target->type == NodeType::LIST) {
            // Global hook
            if (target->List.elements.size() == 0) {
                if (prop->ID.value == "onChange") {
                    if (right->type != NodeType::FUNC) {
                        error_and_exit("onChange hook expects a function");
                    }
                    global_symbol_table->globalHooks_onChange.push_back(right);
                } else if (prop->ID.value == "onCall") {
                    if (right->type != NodeType::FUNC) {
                        error_and_exit("onCall hook expects a function");
                    }
                    global_symbol_table->globalHooks_onCall.push_back(right);
                } else {
                    error_and_exit("Unknown hook '" + prop->ID.value + "'");
                }

                return new_node(NodeType::NONE);
            }

            if (target->List.elements.size() == 1 && target->List.elements[0]->type == NodeType::COMMA_LIST) {
                target = target->List.elements[0];
            }

            for (node_ptr elem : target->List.elements) {
                if (elem->type != NodeType::ID) {
                    error_and_exit("Hook expects either an identifier or a list of identifiers");
                }

                Symbol symbol = get_symbol(elem->ID.value, current_symbol_table);
                if (symbol.value == nullptr) {
                    error_and_exit("Variable '" + elem->ID.value + "' is undefined");
                }

                if (prop->ID.value == "onChange") {
                    if (right->type != NodeType::FUNC) {
                        error_and_exit("onChange hook expects a function");
                    }
                    symbol.value->Hooks.onChange.push_back(right);
                } else if (prop->ID.value == "onCall") {
                    if (right->type != NodeType::FUNC && symbol.value->type != NodeType::FUNC) {
                        error_and_exit("onCall hook expects a function");
                    }
                    symbol.value->Hooks.onCall.push_back(right);
                } else {
                    error_and_exit("Unknown hook '" + prop->ID.value + "'");
                }

                add_symbol(symbol, current_symbol_table);
            }

            return new_node(NodeType::NONE);
        }

        if (target->Operator.value == ".") {
            target = eval_dot(target);
            if (prop->ID.value == "onChange") {
                if (right->type != NodeType::FUNC) {
                    error_and_exit("onChange hook expects a function");
                }
                target->Hooks.onChange.push_back(right);
            } else if (prop->ID.value == "onCall") {
                if (right->type != NodeType::FUNC && target->type != NodeType::FUNC) {
                    error_and_exit("onCall hook expects a function");
                }
                target->Hooks.onCall.push_back(right);
            } else {
                error_and_exit("Unknown hook '" + prop->ID.value + "'");
            }

            return new_node(NodeType::NONE);
        }
    }

    return new_node(NodeType::NONE);
}

// Builtin functions

std::string Interpreter::printable(node_ptr node) {
    switch (node->type) {
        case NodeType::NUMBER: {
            std::string num_str = std::to_string(node->Number.value);
            num_str.erase(num_str.find_last_not_of('0') + 1, std::string::npos);
            num_str.erase(num_str.find_last_not_of('.') + 1, std::string::npos);
            return num_str;
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
            std::string res = "[";
            for (int i = 0; i < node->List.elements.size(); i++) {
                res += printable(node->List.elements[i]);
                if (i < node->List.elements.size()-1) {
                    res += ", ";
                }
            }
            res += "]";
            return res;
        }
        case NodeType::OBJECT: {
            std::string res = "{ ";
            for (auto const& elem : node->Object.properties) {
                res += elem.first + ": " + printable(elem.second) + ' ';
            }
            res += "}";
            return res;
        }
        case NodeType::NONE: {
            return "None";
        }
        case NodeType::ID: {
            return node->ID.value;
        }
        case NodeType::OP: {
            if (node->Operator.value == ".") {
                return printable(node->Operator.left) + "." + printable(node->Operator.right);
            }
            return node->Operator.value;
        }
        case NodeType::ACCESSOR: {
            return printable(node->Accessor.container) + printable(node->Accessor.accessor);
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
    if (node->type == NodeType::FUNC) {
        return eval_function(node);
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
    if (node->type == NodeType::IMPORT) {
        return eval_import(node);
    }
    if (node->type == NodeType::TYPE) {
        return eval_type(node);
    }
    if (node->type == NodeType::OBJECT_DECONSTRUCT) {
        return eval_object_init(node);
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
    if (node->Operator.value == "!") {
        return eval_not(node);
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
    if (node->Operator.value == "+=") {
        return eval_plus_eq(node);
    }
    if (node->Operator.value == "-=") {
        return eval_minus_eq(node);
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

Symbol Interpreter::new_symbol(std::string name, node_ptr value, node_ptr type) {
    Symbol symbol;
    symbol.name = name;
    symbol.value = value;
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

Symbol Interpreter::get_symbol_local(std::string name, std::shared_ptr<SymbolTable> symbol_table) {
    if (symbol_table->symbols.contains(name)) {
        return symbol_table->symbols[name];
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
    node->Meta.is_const = false;
    node->Number.value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_string_node(std::string value) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->Meta.is_const = false;
    node->String.value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_boolean_node(bool value) {
    auto node = std::make_shared<Node>(NodeType::BOOLEAN);
    node->Meta.is_const = false;
    node->Boolean.value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_accessor_node() {
    auto node = std::make_shared<Node>(NodeType::ACCESSOR);
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_node(NodeType type) {
    auto node = std::make_shared<Node>(type);
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_node() {
    auto node = std::make_shared<Node>();
    node->Meta.is_const = false;
    node->line = line;
    node->column = column;
    return node;
}

void Interpreter::error_and_exit(std::string message)
{
    std::string error_message = "Error in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message << "\n";
    while (current_symbol_table->parent != nullptr) {
        std::cout << "In '" + current_symbol_table->filename + "'" << "\n";
        current_symbol_table = current_symbol_table->parent;
    }
    exit(1);
}