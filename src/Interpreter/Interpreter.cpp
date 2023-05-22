#include "Interpreter.hpp"

node_ptr null_value = nullptr;

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

node_ptr Interpreter::eval_const_decl(node_ptr& node) {
    node_ptr existing_symbol = get_symbol_local(node->_Node.ConstantDeclatation().name, current_symbol_table).value;
    if (existing_symbol != nullptr && existing_symbol->type != NodeType::FUNC) {
        return throw_error("Variable '" + node->_Node.ConstantDeclatation().name + "' is already defined");
    }
    node_ptr value = eval_node(node->_Node.ConstantDeclatation().value);
    node_ptr type = eval_node(node->TypeInfo.type);
    value = std::make_shared<Node>(*value);
    value->TypeInfo.type = type;
    if (type && !match_types(type, value)) {
        return throw_error("Variable '" + node->_Node.ConstantDeclatation().name + "' expects a value of type '" + node_repr(type) + "' but was instantiated with value of type '" + node_repr(value) + "'");
    }
    if (!type) {
        type = std::make_shared<Node>(*value);
        type->TypeInfo.is_type = true;
    }
    if (type) {
        value->TypeInfo.type_name = type->TypeInfo.type_name;
    }
    value->Meta.is_const = true;
    if (value->type == NodeType::HOOK) {
        value->_Node.Hook().name = node->_Node.ConstantDeclatation().name;
    }
    if (existing_symbol != nullptr && value->type == NodeType::FUNC) {
        existing_symbol->_Node.Function().dispatch_functions.push_back(value);
        return value;
    }

    if ((value->type == NodeType::OBJECT && value->_Node.Object().is_enum )|| value->TypeInfo.is_type || (value->type == NodeType::LIST && value->_Node.List().is_union)) {
        value->TypeInfo.type_name = node->_Node.ConstantDeclatation().name;
    }

    Symbol symbol = new_symbol(node->_Node.ConstantDeclatation().name, value);
    add_symbol(symbol, current_symbol_table);
    return symbol.value;
}

node_ptr Interpreter::eval_var_decl(node_ptr& node) {
    node_ptr existing_symbol = get_symbol_local(node->_Node.VariableDeclaration().name, current_symbol_table).value;
    if (existing_symbol != nullptr) {
        return throw_error("Variable '" + node->_Node.VariableDeclaration().name + "' is already defined");
    }
    node_ptr value = eval_node(node->_Node.VariableDeclaration().value);
    node_ptr type = eval_node(node->TypeInfo.type);
    if (type && !match_types(type, value)) {
        return throw_error("Variable '" + node->_Node.VariableDeclaration().name + "' expects a value of type '" + node_repr(type) + "' but was instantiated with value of type '" + node_repr(value) + "'");
    }
    if (!type) {
        type = std::make_shared<Node>(*value);
        type->TypeInfo.is_type = true;
    }
    value = std::make_shared<Node>(*value);
    value->TypeInfo.type = type;
    if (type) {
        value->TypeInfo.type_name = type->TypeInfo.type_name;
    }
    value->Meta.is_const = false;
    if (value->type == NodeType::HOOK) {
        value->_Node.Hook().name = node->_Node.VariableDeclaration().name;
    }

    if ((value->type == NodeType::OBJECT && value->_Node.Object().is_enum) || value->TypeInfo.is_type || (value->type == NodeType::LIST && value->_Node.List().is_union)) {
        value->TypeInfo.type_name = node->_Node.VariableDeclaration().name;
    }

    Symbol symbol = new_symbol(node->_Node.VariableDeclaration().name, value);
    add_symbol(symbol, current_symbol_table);
    return symbol.value;
}

node_ptr Interpreter::eval_list(node_ptr& node) {
    node_ptr list = new_node(NodeType::LIST);
    list->TypeInfo = node->TypeInfo;
    node_ptr list_elem_type = new_node(NodeType::ANY);
    list->TypeInfo.type = new_node(NodeType::LIST);
    list->TypeInfo.type->_Node.List().elements.push_back(list_elem_type);

    if (node->_Node.List().elements.size() == 1) {
        if (node->_Node.List().elements[0]->type == NodeType::COMMA_LIST) {
            for (int i = 0; i < node->_Node.List().elements[0]->_Node.List().elements.size(); i++) {
                node_ptr elem = node->_Node.List().elements[0]->_Node.List().elements[i];
                node_ptr evaluated_elem = eval_node(elem);
                evaluated_elem->TypeInfo.is_type = list->TypeInfo.is_type;
                list->_Node.List().elements.push_back(evaluated_elem);

                if (i == 0) {
                    list->TypeInfo.type = new_node(NodeType::LIST);
                    list->TypeInfo.type->TypeInfo.is_type = true;
                    list->TypeInfo.type->_Node.List().elements.push_back(evaluated_elem);
                } else if (i == 1) {
                    if (!match_types(list->TypeInfo.type->_Node.List().elements[0], evaluated_elem)) {
                        list->TypeInfo.type->_Node.List().elements[0] = new_node(NodeType::ANY);
                    }
                }
            }
        } else {
            node_ptr evaluated_elem = eval_node(node->_Node.List().elements[0]);
            if (list->TypeInfo.is_type) {
                evaluated_elem->TypeInfo.is_type = true;
            }
            list->_Node.List().elements.push_back(eval_node(evaluated_elem));
            // If one element, it becomes the type
            list->TypeInfo.type = new_node(NodeType::LIST);
            list->TypeInfo.type->TypeInfo.is_type = true;
            list->TypeInfo.type->_Node.List().elements.push_back(evaluated_elem);
        }
    } else {
        list = node;
    }
    return list;
}

node_ptr Interpreter::eval_object(node_ptr& node) {
    node_ptr object = new_node(NodeType::OBJECT);
    object->TypeInfo = node->TypeInfo;
    // inject current object into scope as "this"
    add_symbol(new_symbol("this", object), current_symbol_table);
    if (node->_Node.Object().elements.size() == 0) {
        return node;
    }
    if (node->_Node.Object().elements[0]->type != NodeType::COMMA_LIST) {
        node_ptr prop = node->_Node.Object().elements[0];
        if (prop->_Node.Op().value != ":") {
            return throw_error("Object must contain properties separated with ':'");
        }
        if (prop->_Node.Op().left->type != NodeType::ID && prop->_Node.Op().left->type != NodeType::STRING) {
            return throw_error("Property names must be identifiers or strings");
        }
        node_ptr value = eval_node(prop->_Node.Op().right);
        value->TypeInfo.is_type = object->TypeInfo.is_type;
        std::string key = prop->_Node.Op().left->type == NodeType::ID ? prop->_Node.Op().left->_Node.ID().value: prop->_Node.Op().left->_Node.String().value;
        // If function exists, add it as dispatch
        if (value->type == NodeType::FUNC && object->_Node.Object().properties.count(key) && object->_Node.Object().properties[key]->type == NodeType::FUNC) {
            object->_Node.Object().properties[key]->_Node.Function().dispatch_functions.push_back(value);
        } else {
            object->_Node.Object().properties[key] = value;
            object->_Node.Object().keys.push_back(key);
            object->_Node.Object().values.push_back(value);
        }
    }
    else {
        for (node_ptr prop : node->_Node.Object().elements[0]->_Node.List().elements) {
            if (prop->type == NodeType::OP && prop->_Node.Op().value != ":") {
                return throw_error("Object must contain properties separated with ':'");
            }
            if (prop->type == NodeType::OP && prop->_Node.Op().left->type != NodeType::ID && prop->_Node.Op().left->type != NodeType::STRING) {
                return throw_error("Property names must be identifiers or strings");
            }
            node_ptr value = eval_node(prop->_Node.Op().right);
            value->TypeInfo.is_type = object->TypeInfo.is_type;
            std::string key = prop->_Node.Op().left->type == NodeType::ID ? prop->_Node.Op().left->_Node.ID().value: prop->_Node.Op().left->_Node.String().value;
            // If function exists, add it as dispatch
            if (value->type == NodeType::FUNC && object->_Node.Object().properties.count(key) && object->_Node.Object().properties[key]->type == NodeType::FUNC) {
                object->_Node.Object().properties[key]->_Node.Function().dispatch_functions.push_back(value);
            } else {
                object->_Node.Object().properties[key] = value;
                object->_Node.Object().keys.push_back(key);
                object->_Node.Object().values.push_back(value);
            }
        }
    }
    // remove "this" from scope
    delete_symbol("this", current_symbol_table);
    return object;
}

node_ptr Interpreter::eval_func_call(node_ptr& node, node_ptr func) {
    node_ptr function = new_node(NodeType::FUNC);

    if (!func) {
        if (node->_Node.FunctionCall().name == "print") {
            if (node->_Node.FunctionCall().args.size() == 1) {
                node_ptr arg = eval_node(node->_Node.FunctionCall().args[0]);
                builtin_print(arg);
            } else {
                for (node_ptr arg : node->_Node.FunctionCall().args) {
                    node_ptr evaluated_arg = eval_node(arg);
                    builtin_print(evaluated_arg);
                    std::cout << '\n';
                }
            }
            return new_node(NodeType::NONE);
        }
        if (node->_Node.FunctionCall().name == "println") {
            if (node->_Node.FunctionCall().args.size() == 1) {
                node_ptr arg = eval_node(node->_Node.FunctionCall().args[0]);
                builtin_print(arg);
                std::cout << "\n";
            } else {
                for (node_ptr arg : node->_Node.FunctionCall().args) {
                    node_ptr evaluated_arg = eval_node(arg);
                    builtin_print(evaluated_arg);
                    std::cout << '\n';
                }
            }
            return new_node(NodeType::NONE);
        }
        if (node->_Node.FunctionCall().name == "error") {
            if (node->_Node.FunctionCall().args.size() != 1) {
                return throw_error("Function " + node->_Node.FunctionCall().name + " expects 1 argument");
            }
            node_ptr arg = eval_node(node->_Node.FunctionCall().args[0]);
            std::string message = printable(arg);
            line = node->line;
            column = node->column;
            return throw_error(message);
        }
        if (node->_Node.FunctionCall().name == "string") {
            if (node->_Node.FunctionCall().args.size() != 1) {
                return throw_error("Function " + node->_Node.FunctionCall().name + " expects 1 argument");
            }
            node_ptr arg = eval_node(node->_Node.FunctionCall().args[0]);
            return new_string_node(printable(arg));
        }
        if (node->_Node.FunctionCall().name == "number") {
            if (node->_Node.FunctionCall().args.size() != 1) {
                return throw_error("Function " + node->_Node.FunctionCall().name + " expects 1 argument");
            }
            node_ptr var = eval_node(node->_Node.FunctionCall().args[0]);

            switch(var->type) {
                case NodeType::NONE: new_number_node(0);
                case NodeType::NUMBER: return node;
                case NodeType::STRING: {
                    try {
                        return new_number_node(std::stod(var->_Node.String().value));
                    } catch(...) {
                        return throw_error("Cannot convert \"" + var->_Node.String().value + "\" to a number");
                    }
                };
                case NodeType::BOOLEAN: {
                    if (var->_Node.Boolean().value) {
                        return new_number_node(1);
                    }

                    return new_number_node(0);
                };
                default: return new_string_node("None");
            }
        }
        if (node->_Node.FunctionCall().name == "type") {
            if (node->_Node.FunctionCall().args.size() != 1) {
                return throw_error("Function " + node->_Node.FunctionCall().name + " expects 1 argument");
            }

            node_ptr var = eval_node(node->_Node.FunctionCall().args[0]);

            switch(var->type) {
                case NodeType::NONE: return new_string_node("None");
                case NodeType::NUMBER: return new_string_node("Number");
                case NodeType::STRING: return new_string_node("String");
                case NodeType::BOOLEAN: return new_string_node("Boolean");
                case NodeType::LIST: return new_string_node("List");
                case NodeType::OBJECT: return new_string_node("Object");
                case NodeType::FUNC: return new_string_node("Function");
                case NodeType::POINTER: return new_string_node("Pointer");
                case NodeType::LIB: return new_string_node("Library");
                case NodeType::ANY: return new_string_node("Any");
                default: return new_string_node("None");
            }
        }
        if (node->_Node.FunctionCall().name == "load_lib") {
            return eval_load_lib(node);
        }
        if (node->_Node.FunctionCall().name == "exit") {
            if (node->_Node.FunctionCall().args.size() != 1) {
                exit(1);
            }

            node_ptr status_code = eval_node(node->_Node.FunctionCall().args[0]);

            if (status_code->type != NodeType::NUMBER) {
                exit(1);
            } else {
                exit(status_code->_Node.Number().value);
            }
        }
    }

    if (func != nullptr) {
        function->_Node.Function().name = func->_Node.Function().name;
        function->_Node.Function().args = std::vector<node_ptr>(func->_Node.Function().args);
        function->_Node.Function().params = std::vector<node_ptr>(func->_Node.Function().params);
        function->_Node.Function().body = func->_Node.Function().body;
        function->_Node.Function().closure = func->_Node.Function().closure;
        function->_Node.Function().is_hook = func->_Node.Function().is_hook;
        function->_Node.Function().decl_filename = func->_Node.Function().decl_filename;
        function->Hooks.onCall = func->Hooks.onCall;
        function->_Node.Function().param_types = func->_Node.Function().param_types;
        function->_Node.Function().return_type = func->_Node.Function().return_type;
        function->_Node.Function().dispatch_functions = func->_Node.Function().dispatch_functions;
    } else if (node->_Node.FunctionCall().caller == nullptr) {
        Symbol function_symbol = get_symbol(node->_Node.FunctionCall().name, current_symbol_table);
        if (function_symbol.value == nullptr) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' is undefined");
        }
        if (function_symbol.value->type != NodeType::FUNC) {
            return throw_error("Variable '" + node->_Node.FunctionCall().name + "' is not a function");
        }
        function->_Node.Function().name = function_symbol.name;
        function->_Node.Function().args = std::vector<node_ptr>(function_symbol.value->_Node.Function().args);
        function->_Node.Function().params = std::vector<node_ptr>(function_symbol.value->_Node.Function().params);
        function->_Node.Function().body = function_symbol.value->_Node.Function().body;
        function->_Node.Function().closure = function_symbol.value->_Node.Function().closure;
        function->_Node.Function().is_hook = function_symbol.value->_Node.Function().is_hook;
        function->_Node.Function().decl_filename = function_symbol.value->_Node.Function().decl_filename;
        function->Hooks.onCall = function_symbol.value->Hooks.onCall;
        function->_Node.Function().param_types = function_symbol.value->_Node.Function().param_types;
        function->_Node.Function().return_type = function_symbol.value->_Node.Function().return_type;
        function->_Node.Function().dispatch_functions = function_symbol.value->_Node.Function().dispatch_functions;
    } else {
        node_ptr method = node->_Node.FunctionCall().caller->_Node.Object().properties[node->_Node.FunctionCall().name];
        if (method == nullptr) {
            return throw_error("Method '" + node->_Node.FunctionCall().name + "' does not exist");
        }
        if (method->type != NodeType::FUNC) {
            return throw_error("Variable '" + node->_Node.FunctionCall().name + "' is not a function");
        }
        function->_Node.Function().name = method->_Node.Function().name;
        function->_Node.Function().args = std::vector<node_ptr>(method->_Node.Function().args);
        function->_Node.Function().params = std::vector<node_ptr>(method->_Node.Function().params);
        function->_Node.Function().body = method->_Node.Function().body;
        function->_Node.Function().closure = method->_Node.Function().closure;
        function->_Node.Function().is_hook = method->_Node.Function().is_hook;
        function->_Node.Function().decl_filename = method->_Node.Function().decl_filename;
        function->Hooks.onCall = method->Hooks.onCall;
        function->_Node.Function().param_types = method->_Node.Function().param_types;
        function->_Node.Function().return_type = method->_Node.Function().return_type;
        function->_Node.Function().dispatch_functions = method->_Node.Function().dispatch_functions;
    }

    std::vector<node_ptr> args;
    for (node_ptr arg : node->_Node.FunctionCall().args) {
        args.push_back(eval_node(arg));
    }

    // Check if function args match any multiple dispatch functions

    auto functions = std::vector<node_ptr>(function->_Node.Function().dispatch_functions);
    functions.insert(functions.begin(), function);

    bool func_match = false;

    for (node_ptr& fx: functions) {

        // If function param size does not match args size, skip
        if (fx->_Node.Function().params.size() != args.size()) {
            continue;
        }

        for (int i = 0; i < args.size(); i++) {
            // node_ptr param = function->_Node.Function().params[i];
            node_ptr param = fx->_Node.Function().params[i];
            node_ptr param_type = fx->_Node.Function().param_types[param->_Node.ID().value];
            if (param_type) {
                if (!match_types(param_type, args[i])) {
                    goto not_found;
                }
            } else {
                continue;
            }
        }

        // If we get here, we've found the function
        function = std::make_shared<Node>(*fx);
        func_match = true;
        break;

        not_found:
            continue;
    }

    if (!func_match) {
        std::string argsStr = "(";
        for (int i = 0; i < args.size(); i++) {
            argsStr += node_repr(args[i]);
            if (i != args.size()-1) {
                argsStr += ", ";
            }
        }
        argsStr += ")";
        return throw_error("Dispatch error in function '" + node->_Node.FunctionCall().name + "' - No function found matching args: " + argsStr + "\n\nAvailable functions:\n\n" + printable(functions[0]));
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

    if (node->_Node.FunctionCall().caller != nullptr) {
        current_scope->symbols["this"] = new_symbol("this", node->_Node.FunctionCall().caller);
    }

    current_symbol_table = current_scope;
    current_symbol_table->filename = function->_Node.Function().name + ": " + function->_Node.Function().decl_filename;
    
    // Inject closure into local scope

    for (auto& elem : function->_Node.Function().closure) {
        add_symbol(new_symbol(elem.first, elem.second), local_scope);
    }

    int num_empty_args = std::count(function->_Node.Function().args.begin(), function->_Node.Function().args.end(), nullptr);

    if (args.size() > num_empty_args) {
        return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_empty_args) + " parameters but " + std::to_string(args.size()) + " were provided");
    }

    for (int i = 0; i < args.size(); i++) {
        std::string name = function->_Node.Function().params[i]->_Node.ID().value;
        node_ptr value = args[i];
        Symbol symbol = new_symbol(name, value);
        add_symbol(symbol, current_symbol_table);
        function->_Node.Function().args[i] = value;
    }

    node_ptr res = function;

    if (function->_Node.Function().body->type != NodeType::OBJECT) {
        res = eval_node(function->_Node.Function().body);
    } else {
        for (int i = 0; i < function->_Node.Function().body->_Node.Object().elements.size(); i++) {
            node_ptr expr = function->_Node.Function().body->_Node.Object().elements[i];
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                if (evaluated_expr->_Node.Return().value == nullptr) {
                    res = new_node(NodeType::NONE);
                } else {
                    res = eval_node(evaluated_expr->_Node.Return().value);
                }
                break;
            } else if (i == function->_Node.Function().body->_Node.Object().elements.size()-1) {
                res = evaluated_expr;
                if (res->type == NodeType::RETURN) {
                    if (res->_Node.Return().value == nullptr) {
                        res = new_node(NodeType::NONE);
                    } else {
                        res = res->_Node.Return().value;
                    }
                }
                break;
            }
        }
    }

    // Check against return type
    if (function->_Node.Function().return_type) {
        if (!match_types(function->_Node.Function().return_type, res)) {
            return throw_error("Type Error in '" + function->_Node.Function().name + "': Return type does not match defined return type");
        }
    }

    auto allOnCallFunctionsLists = {std::cref(function->Hooks.onCall), std::cref(global_symbol_table->globalHooks_onCall)};

    if (!function->_Node.Function().is_hook) {      
        for (const auto& function_list : allOnCallFunctionsLists) {
            for (node_ptr func : function_list.get()) {
                node_ptr function_call = new_node(NodeType::FUNC_CALL);
                function_call->_Node.FunctionCall().name = func->_Node.Function().name;
                function_call->_Node.FunctionCall().args = std::vector<node_ptr>();
                if (func->_Node.Function().params.size() > 0) {
                    node_ptr file_info = new_node(NodeType::OBJECT);
                    file_info->_Node.Object().properties["name"] = new_string_node(function->_Node.Function().name);
                    node_ptr args_list = new_node(NodeType::LIST);
                    for (node_ptr arg : args) {
                        args_list->_Node.List().elements.push_back(arg);
                    }
                    file_info->_Node.Object().properties["args"] = args_list;
                    file_info->_Node.Object().properties["result"] = res;
                    file_info->_Node.Object().properties["filename"] = new_string_node(file_name);
                    file_info->_Node.Object().properties["line"] = new_number_node(line);
                    file_info->_Node.Object().properties["column"] = new_number_node(column);
                    function_call->_Node.FunctionCall().args.push_back(file_info);
                }
                eval_func_call(function_call, func);
            }
        }
    }

    current_symbol_table = current_symbol_table->parent->parent;
    current_symbol_table->child->child = nullptr;
    current_symbol_table->child = nullptr;

    return res;
}

node_ptr Interpreter::eval_if_statement(node_ptr& node) {
    node_ptr conditional = eval_node(node->_Node.IfStatement().condition);
    bool is_true = false;

    if (conditional->type == NodeType::BOOLEAN) {
        is_true = conditional->_Node.Boolean().value;
    } else {
        is_true = conditional->type != NodeType::NONE;
    }

    if (is_true) {
        for (node_ptr expr : node->_Node.IfStatement().body->_Node.Object().elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                evaluated_expr->_Node.Return().value = eval_node(evaluated_expr->_Node.Return().value);
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

node_ptr Interpreter::eval_if_block(node_ptr& node) {
    for (node_ptr statement : node->_Node.IfBlock().statements) {
        if (statement->type == NodeType::IF_STATEMENT) {
            node_ptr conditional = eval_node(statement->_Node.IfStatement().condition);
            if (conditional->_Node.Boolean().value) {
                return eval_node(statement);
            }
        } else if (statement->type == NodeType::OBJECT) {
            for (node_ptr expr : statement->_Node.Object().elements) {
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

node_ptr Interpreter::eval_return(node_ptr& node) {
    node_ptr ret = new_node(NodeType::RETURN);
    if (ret->_Node.Return().value == nullptr) {
        ret->_Node.Return().value = new_node(NodeType::NONE);
    } else {
        ret->_Node.Return().value = eval_node(node->_Node.Return().value);
    }
    return ret;
}

node_ptr Interpreter::eval_for_loop(node_ptr& node) {

    if (node->_Node.ForLoop().iterator != nullptr) {
        node_ptr iterator = eval_node(node->_Node.ForLoop().iterator);

        // TODO: Implement for object looping

        if (iterator->type != NodeType::LIST) {
            return throw_error("For loop iterator must be a list");
        }

        auto current_scope = current_symbol_table;

        auto loop_symbol_table = std::make_shared<SymbolTable>();
        
        for (int i = 0; i < iterator->_Node.List().elements.size(); i++) {

            loop_symbol_table->symbols.clear();
            current_symbol_table = loop_symbol_table;
            current_symbol_table->parent = current_scope;

            if (node->_Node.ForLoop().index_name) {
                node_ptr number_node = new_number_node(i);
                add_symbol(new_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, number_node), current_symbol_table);
            }
            if (node->_Node.ForLoop().value_name) {
                add_symbol(new_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, iterator->_Node.List().elements[i]), current_symbol_table);
            }
            for (node_ptr expr : node->_Node.ForLoop().body->_Node.Object().elements) {
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
                continue;
            _break:
                current_symbol_table = current_scope;
                break;
        }

        // Cleanup

        if (node->_Node.ForLoop().index_name != nullptr) {
            delete_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, current_symbol_table);
        }
        if (node->_Node.ForLoop().value_name != nullptr) {
            delete_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, current_symbol_table);
        }

        current_symbol_table = current_scope;

        return new_node(NodeType::NONE);
    }

    node_ptr start_node = eval_node(node->_Node.ForLoop().start);
    node_ptr end_node = eval_node(node->_Node.ForLoop().end);

    if (start_node->type != NodeType::NUMBER && end_node->type != NodeType::NUMBER) {
        return throw_error("For loop range expects two numbers");
    }

    int start = start_node->_Node.Number().value;
    int end = end_node->_Node.Number().value;

    auto current_scope = current_symbol_table;

    int for_index = -1;

    auto loop_symbol_table = std::make_shared<SymbolTable>();
    current_symbol_table = loop_symbol_table;
    current_symbol_table->parent = current_scope;

    for (int i = start; i < end; i++) {

        for_index++;

        loop_symbol_table->symbols.clear();

        int index = i - start_node->_Node.Number().value;
        if (node->_Node.ForLoop().index_name) {
            node_ptr index_node = new_number_node(for_index);
            current_symbol_table->symbols[node->_Node.ForLoop().index_name->_Node.ID().value] = new_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, index_node);
            // add_symbol(new_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, new_number_node(for_index)), current_symbol_table);
        }
        if (node->_Node.ForLoop().value_name) {
            node_ptr value_node = new_number_node(i);
            current_symbol_table->symbols[node->_Node.ForLoop().value_name->_Node.ID().value] = new_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, value_node);
            // add_symbol(new_symbol(
            //     node->_Node.ForLoop().value_name->_Node.ID().value, new_number_node(i)), current_symbol_table
            // );
        }
        for (node_ptr& expr : node->_Node.ForLoop().body->_Node.Object().elements) {
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
            continue;
        _break2:
            current_symbol_table = current_scope;
            break;
    }

    // Cleanup

    if (node->_Node.ForLoop().index_name != nullptr) {
        delete_symbol(node->_Node.ForLoop().index_name->_Node.ID().value, current_symbol_table);
    }
    if (node->_Node.ForLoop().value_name != nullptr) {
        delete_symbol(node->_Node.ForLoop().value_name->_Node.ID().value, current_symbol_table);
    }

    current_symbol_table = current_scope;

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_while_loop(node_ptr& node) {
    node_ptr conditional = eval_node(node->_Node.WhileLoop().condition);
    if (conditional->type != NodeType::BOOLEAN) {
        return throw_error("While loop conditional must evaluate to a boolean");
    }

    auto current_scope = current_symbol_table;

    while (conditional->_Node.Boolean().value) {
        auto loop_symbol_table = std::make_shared<SymbolTable>();
        for (auto& symbol : current_symbol_table->symbols) {
            loop_symbol_table->symbols[symbol.first] = symbol.second;
        }

        current_symbol_table = loop_symbol_table;
        current_symbol_table->parent = current_scope;
        
        for (node_ptr expr : node->_Node.WhileLoop().body->_Node.Object().elements) {
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::RETURN) {
                current_symbol_table = current_scope;
                return evaluated_expr->_Node.Return().value;
            }
            if (evaluated_expr->type == NodeType::BREAK) {
                goto _break;
            }
        }

        conditional = eval_node(node->_Node.WhileLoop().condition);

        current_symbol_table = current_scope;
    }

    _break:
        current_symbol_table = current_scope;

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_accessor(node_ptr& node) {
    node_ptr container = eval_node(node->_Node.Accessor().container);
    node_ptr accessor = eval_node(node->_Node.Accessor().accessor);

    if (accessor->_Node.List().elements.size() != 1) {
        return throw_error("Malformed accessor");
    }

    if (container->type == NodeType::STRING) {
        node_ptr index_node = accessor->_Node.List().elements[0];
        if (index_node->type != NodeType::NUMBER) {
            return throw_error("List accessor expects a number");
        }
        int index = index_node->_Node.Number().value;
        if (index >= container->_Node.String().value.length() || index < 0) {
            return new_node(NodeType::NONE);
        }
        return new_string_node(std::string(1, container->_Node.String().value[index]));
    }

    if (container->type == NodeType::LIST) {
        node_ptr index_node = accessor->_Node.List().elements[0];
        if (index_node->type != NodeType::NUMBER) {
            return throw_error("List accessor expects a number");
        }
        int index = index_node->_Node.Number().value;
        if (index >= container->_Node.List().elements.size() || index < 0) {
            return new_node(NodeType::NONE);
        }
        return container->_Node.List().elements[index];
    }

    if (container->type == NodeType::OBJECT) {
        node_ptr prop_node = accessor->_Node.List().elements[0];
        if (prop_node->type != NodeType::STRING) {
            return throw_error("Object accessor expects a string");
        }
        std::string prop = prop_node->_Node.String().value;
        if (container->_Node.Object().properties.contains(prop)) {
            return container->_Node.Object().properties[prop];
        }
        return new_node(NodeType::NONE);
    }

    return throw_error("Value of type '" + node_repr(container) + "' is not accessable");
}

node_ptr Interpreter::eval_function(node_ptr& node) {
    node->_Node.Function().decl_filename = file_name;
    if (node->_Node.Function().return_type) {
        node->_Node.Function().return_type = eval_node(node->_Node.Function().return_type);
        if (node->_Node.Function().return_type->type == NodeType::LIST) {
            node->_Node.Function().return_type->TypeInfo.is_type = true;
        } else if (node->_Node.Function().return_type->type == NodeType::OBJECT) {
            node->_Node.Function().return_type->TypeInfo.is_type = true;
        }
    }
    // Go through params to see if they are typed, and store their types
    for (auto& param : node->_Node.Function().params) {
        if (param->type == NodeType::OP && param->_Node.Op().value == ":") {
            node_ptr param_type = eval_node(param->_Node.Op().right);
            if (param_type->type == NodeType::LIST) {
                param_type->TypeInfo.is_type = true;
            } else if (param_type->type == NodeType::OBJECT) {
                param_type->TypeInfo.is_type = true;
            }
            node->_Node.Function().param_types[param->_Node.Op().left->_Node.ID().value] 
                = param_type;
            param = param->_Node.Op().left;
        }
    }
    // Inject current scope as closure
    for (auto& symbol : current_symbol_table->symbols) {
        node->_Node.Function().closure[symbol.first] = symbol.second.value;
    }
    return node;
}

node_ptr Interpreter::eval_enum(node_ptr& node) {
    node_ptr enum_object = eval_object(node->_Node.Enum().body);
    enum_object->Meta.is_const = true;
    enum_object->_Node.Object().is_enum = true;
    enum_object->TypeInfo.type_name = node->_Node.Enum().name;
    Symbol symbol = new_symbol(node->_Node.Enum().name, enum_object);
    add_symbol(symbol, current_symbol_table);
    return enum_object;
}

node_ptr Interpreter::eval_union(node_ptr& node) {
    node_ptr union_body = node->_Node.Union().body;
    node_ptr union_list = new_node(NodeType::LIST);
    if (union_body->_Node.Object().elements.size() == 0) {
        return throw_error("Union cannot be empty");
    }
    if (union_body->_Node.Object().elements.size() == 1 && union_body->_Node.Object().elements[0]->type == NodeType::COMMA_LIST) {
        union_body = union_body->_Node.Object().elements[0];
        for (node_ptr elem : union_body->_Node.List().elements) {
            union_list->_Node.List().elements.push_back(eval_node(elem));
        }
    } else {
        union_list->_Node.List().elements.push_back(eval_node(union_body->_Node.Object().elements[0]));
    }

    union_list->Meta.is_const = true;
    union_list->_Node.List().is_union = true;
    union_list->TypeInfo.type_name = node->_Node.Union().name;
    union_list->TypeInfo.is_type = true;
    Symbol symbol = new_symbol(node->_Node.Union().name, union_list);
    add_symbol(symbol, current_symbol_table);
    return union_list;
}

node_ptr Interpreter::eval_type(node_ptr& node) {

    if (node->_Node.Type().expr) {
        node_ptr val = std::make_shared<Node>(*node->_Node.Type().expr);
        val = eval_node(val);
        Symbol symbol = new_symbol(node->_Node.Type().name, val);
        // Check if type is function and if it returns a type
        // If it does, it's a type
        // Otherwise, it's a refinement type
        if (symbol.value->type == NodeType::FUNC) {
            symbol.value->_Node.Function().name = node->_Node.Type().name;
            if (symbol.value->_Node.Function().return_type && symbol.value->_Node.Function().return_type->TypeInfo.is_type) {
                symbol.value->TypeInfo.is_refinement_type = false;
            } else {
                symbol.value->TypeInfo.is_refinement_type = true;
            }
        } else {
            symbol.value->TypeInfo.is_literal_type = true;
        }
        add_symbol(symbol, current_symbol_table);
        return symbol.value;
    }

    node_ptr object = new_node(NodeType::OBJECT);

    if (node->_Node.Type().body->_Node.Object().elements[0]->type != NodeType::COMMA_LIST) {
        node_ptr prop = node->_Node.Type().body->_Node.Object().elements[0];
        node_ptr def_val = nullptr;

        if (prop->type == NodeType::ID) {
            // This is essentually an any type
            object->_Node.Object().properties[prop->_Node.ID().value] = new_node(NodeType::ANY);
            object->_Node.Object().properties[prop->_Node.ID().value]->Meta.is_untyped_property = true;
            object->_Node.Object().defaults[prop->_Node.ID().value] = new_node(NodeType::ANY);
            goto end;
        }

        if (prop->_Node.Op().value == "=" && prop->_Node.Op().left->type == NodeType::ID) {
            // This is essentually an any type
            std::string name = prop->_Node.Op().left->_Node.ID().value;
            node_ptr value = eval_node(prop->_Node.Op().right);

            object->_Node.Object().properties[name] = new_node(NodeType::ANY);
            object->_Node.Object().properties[name]->Meta.is_untyped_property = true;
            if (value->type == NodeType::FUNC && object->_Node.Object().defaults.count(name) && object->_Node.Object().defaults[name]->type == NodeType::FUNC) {
                object->_Node.Object().defaults[name]->_Node.Function().dispatch_functions.push_back(value);
            } else {
                object->_Node.Object().defaults[name] = value;
            }
            goto end;
        }

        if (prop->_Node.Op().value == "=") {
            def_val = eval_node(prop->_Node.Op().right);
            prop = prop->_Node.Op().left;
        }

        if (prop->_Node.Op().value != ":") {
            return throw_error("Object must contain properties separated with ':'");
        }
        if (prop->_Node.Op().left->type != NodeType::ID) {
            return throw_error("Property names must be identifiers");
        }

        node_ptr value = prop->_Node.Op().right;

        // Check if container type (List or Obj) and tag it as a type
        node_ptr type_val = eval_node(prop->_Node.Op().right);
        if (type_val->type == NodeType::LIST) {
            if (type_val->_Node.List().elements.size() > 1) {
                return throw_error("List types can only contain one type");
            }
            type_val->TypeInfo.is_type = true;
        } else if (type_val->type == NodeType::OBJECT) {
            type_val->TypeInfo.is_type = true;
        }

        std::string name = prop->_Node.Op().left->_Node.ID().value;

        object->_Node.Object().properties[name] = type_val;
        if (def_val) {
            if (!match_types(type_val, def_val)) {
                return throw_error("Default type constructor for propery '" + name + "' does not match type: Expecting value of type '" + node_repr(type_val) + "' but received value of type '" + node_repr(def_val) + "'");
            }
            if (def_val->type == NodeType::FUNC && object->_Node.Object().defaults.count(name) && object->_Node.Object().defaults[name]->type == NodeType::FUNC) {
                object->_Node.Object().defaults[name]->_Node.Function().dispatch_functions.push_back(def_val);
            } else {
                object->_Node.Object().defaults[name] = def_val;
            }
        }

    } else {
        for (node_ptr prop : node->_Node.Type().body->_Node.Object().elements[0]->_Node.List().elements) {

            node_ptr def_val = nullptr;

            if (prop->type == NodeType::ID) {
                // This is essentually an any type
                object->_Node.Object().properties[prop->_Node.ID().value] = new_node(NodeType::ANY);
                object->_Node.Object().properties[prop->_Node.ID().value]->Meta.is_untyped_property = true;
                object->_Node.Object().defaults[prop->_Node.ID().value] = new_node(NodeType::ANY);
                continue;
            }

            if (prop->_Node.Op().value == "=" && prop->_Node.Op().left->type == NodeType::ID) {
                // This is essentually an any type
                std::string name = prop->_Node.Op().left->_Node.ID().value;
                node_ptr value = eval_node(prop->_Node.Op().right);

                object->_Node.Object().properties[name] = new_node(NodeType::ANY);
                object->_Node.Object().properties[name]->Meta.is_untyped_property = true;
                if (value->type == NodeType::FUNC && object->_Node.Object().defaults.count(name) && object->_Node.Object().defaults[name]->type == NodeType::FUNC) {
                    object->_Node.Object().defaults[name]->_Node.Function().dispatch_functions.push_back(value);
                } else {
                    object->_Node.Object().defaults[name] = value;
                }
                // object->_Node.Object().properties[prop->_Node.Op().left->_Node.ID().value] = new_node(NodeType::NONE);
                // object->_Node.Object().properties[prop->_Node.Op().left->_Node.ID().value]->Meta.is_untyped_property = true;
                // object->_Node.Object().defaults[prop->_Node.Op().left->_Node.ID().value] = eval_node(prop->_Node.Op().right);
                continue;
            }

            if (prop->_Node.Op().value == "=") {
                def_val = eval_node(prop->_Node.Op().right);
                prop = prop->_Node.Op().left;
            }

            if (prop->_Node.Op().value != ":") {
                return throw_error("Object must contain properties separated with ':'");
            }
            if (prop->_Node.Op().left->type != NodeType::ID) {
                return throw_error("Property names must be identifiers");
            }
            
            node_ptr value = prop->_Node.Op().right;

            std::string name = prop->_Node.Op().left->_Node.ID().value;

            // Check if container type (List or Obj) and tag it as a type
            node_ptr type_val = eval_node(prop->_Node.Op().right);
            if (type_val->type == NodeType::LIST) {
                if (type_val->_Node.List().elements.size() > 1) {
                    return throw_error("List types can only contain one type");
                }
                type_val->TypeInfo.is_type = true;
            } else if (type_val->type == NodeType::OBJECT) {
                type_val->TypeInfo.is_type = true;
            }
            object->_Node.Object().properties[name] = type_val;
            if (def_val) {
                if (!match_types(type_val, def_val)) {
                    return throw_error("Default type constructor for propery '" + name + "' does not match type: Expecting value of type '" + node_repr(type_val) + "' but received value of type '" + node_repr(def_val) + "'");
                }
                if (def_val->type == NodeType::FUNC && object->_Node.Object().defaults.count(name) && object->_Node.Object().defaults[name]->type == NodeType::FUNC) {
                    object->_Node.Object().defaults[name]->_Node.Function().dispatch_functions.push_back(def_val);
                } else {
                    object->_Node.Object().defaults[name] = def_val;
                }
                //object->_Node.Object().defaults[name] = def_val;
            }
        }
    }

    end:

    object->TypeInfo.is_type = true;
    object->TypeInfo.type_name = node->_Node.Type().name;
    Symbol symbol = new_symbol(node->_Node.Type().name, object);
    add_symbol(symbol, current_symbol_table);
    return object;
}

node_ptr Interpreter::eval_type_ext(node_ptr& node) {
    node_ptr type = eval_node(node->_Node.TypeExt().type);
    node_ptr body = eval_object(node->_Node.TypeExt().body);

    // Check that all property values resolve to functions
    for (auto& prop : body->_Node.Object().properties) {
        if (prop.second->type != NodeType::FUNC) {
            return throw_error("Type extension properties must resolve to functions");
        }
    }

    if (type->type == NodeType::STRING) {
        global_symbol_table->StringExtensions.insert(body->_Node.Object().properties.begin(), body->_Node.Object().properties.end());
        return new_node(NodeType::NONE);
    }
    if (type->type == NodeType::NUMBER) {
        global_symbol_table->NumberExtensions.insert(body->_Node.Object().properties.begin(), body->_Node.Object().properties.end());
        return new_node(NodeType::NONE);
    }
    if (type->type == NodeType::BOOLEAN) {
        global_symbol_table->BoolExtensions.insert(body->_Node.Object().properties.begin(), body->_Node.Object().properties.end());
        return new_node(NodeType::NONE);
    }
    if (type->type == NodeType::LIST) {
        global_symbol_table->ListExtensions.insert(body->_Node.Object().properties.begin(), body->_Node.Object().properties.end());
        return new_node(NodeType::NONE);
    }
    if (type->type == NodeType::OBJECT) {
        if (type->TypeInfo.is_type) {
            if (global_symbol_table->CustomTypeExtensions.count(type->TypeInfo.type_name)) {
                global_symbol_table->CustomTypeExtensions[type->TypeInfo.type_name].insert(body->_Node.Object().properties.begin(), body->_Node.Object().properties.end());
            } else {
                global_symbol_table->CustomTypeExtensions[type->TypeInfo.type_name] = std::unordered_map<std::string, node_ptr>();
                global_symbol_table->CustomTypeExtensions[type->TypeInfo.type_name].insert(body->_Node.Object().properties.begin(), body->_Node.Object().properties.end());
            }
            return new_node(NodeType::NONE);
        }

        global_symbol_table->ObjectExtensions.insert(body->_Node.Object().properties.begin(), body->_Node.Object().properties.end());
        return new_node(NodeType::NONE);
    }
    if (type->type == NodeType::FUNC) {
        global_symbol_table->FunctionExtensions.insert(body->_Node.Object().properties.begin(), body->_Node.Object().properties.end());
        return new_node(NodeType::NONE);
    }
    if (type->type == NodeType::NONE) {
        global_symbol_table->NoneExtensions.insert(body->_Node.Object().properties.begin(), body->_Node.Object().properties.end());
        return new_node(NodeType::NONE);
    }

    return throw_error("Malformed Type extension");
    return new_node(NodeType::NONE);
}

bool Interpreter::match_values(node_ptr& nodeA, node_ptr& nodeB) {
    if (nodeA->type != nodeB->type) {
        return false;
    }

    if (nodeA->type == NodeType::NONE) {
        return true;
    }
    
    if (nodeA->type == NodeType::NUMBER) {
        return nodeA->_Node.Number().value == nodeB->_Node.Number().value;
    }
    if (nodeA->type == NodeType::BOOLEAN) {
        return nodeA->_Node.Boolean().value == nodeB->_Node.Boolean().value;
    }
    if (nodeA->type == NodeType::STRING) {
        return nodeA->_Node.String().value == nodeB->_Node.String().value;
    }
    if (nodeA->type == NodeType::LIST) {
        if (nodeA->_Node.List().elements.size() != nodeB->_Node.List().elements.size()) {
            return false;
        }

        for (int i = 0; i < nodeA->_Node.List().elements.size(); i++) {
            if (!match_values(nodeA->_Node.List().elements[i], nodeB->_Node.List().elements[i])) {
                return false;
            }
        }
        return true;
    }
    if (nodeA->type == NodeType::OBJECT) {
        if (nodeA->_Node.Object().properties.size() != nodeB->_Node.Object().properties.size()) {
            return false;
        }

        for (auto& prop : nodeA->_Node.Object().properties) {
            if (!match_values(nodeA->_Node.Object().properties[prop.first], nodeB->_Node.Object().properties[prop.first])) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Interpreter::match_types(node_ptr& _type, node_ptr& _value) {
    node_ptr type = std::make_shared<Node>(*_type);
    if (!type->TypeInfo.is_type && type->TypeInfo.type) {
        type = type->TypeInfo.type;
    }
    node_ptr value = std::make_shared<Node>(*_value);

    if (type == nullptr || value == nullptr) {
        return false;
    }

    if (type->type == NodeType::ANY) {
        return true;
    }

    if (type->type == NodeType::FUNC && type->TypeInfo.is_refinement_type) {
        node_ptr func_call = new_node(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().name = type->_Node.Function().name;
        func_call->_Node.FunctionCall().args.push_back(value);
        node_ptr res = eval_func_call(func_call, type);
        if (res->type != NodeType::BOOLEAN) {
            error_and_exit("Refinement types must return a boolean value");
        }
        return res->_Node.Boolean().value;
    }

    if (type->type == NodeType::LIST && type->_Node.List().is_union) {
        for (node_ptr& type : type->_Node.List().elements) {
            if (match_types(type, value)) {
                if (!type->TypeInfo.is_type) {
                    if (match_values(type, value)) {
                        return true;
                    } else {
                        continue;
                    }
                }
                return true;
            }
        }
        return false;
    }

    if (type->type == NodeType::OBJECT && type->_Node.Object().is_enum) {
        for (auto& elem : type->_Node.Object().properties) {
            if (match_values(elem.second, value)) {
                return true;
            }
        }

        return false;
    }

    if (type->TypeInfo.is_literal_type) {
        return match_values(type, value);
    }

    if (type->type != value->type) {
        return false;
    }

    if (type->type == NodeType::LIST) {

        if (type->TypeInfo.is_type) {
            if (type->_Node.List().elements.size() == 0) {
                return true;
            }
            node_ptr list_type = type->_Node.List().elements[0];

            for (node_ptr& elem : value->_Node.List().elements) {
                if (!match_types(list_type, elem)) {
                    return false;
                }
            }

            return true;
        } else if (value->TypeInfo.is_type) {
            if (value->_Node.List().elements.size() == 0) {
                return true;
            }
            node_ptr list_type = value->_Node.List().elements[0];

            for (node_ptr& elem : type->_Node.List().elements) {
                if (!match_types(list_type, elem)) {
                    return false;
                }
            }

            return true;
        } else {
            return false;
        }
    }

    if (type->type == NodeType::OBJECT) {
        if (type->TypeInfo.type_name != value->TypeInfo.type_name) {
            return false;
        }

        if (type->TypeInfo.is_type && type->TypeInfo.type_name == "" && value->TypeInfo.type_name == "") {
            return true;
        }

        if (value->TypeInfo.is_type && value->TypeInfo.type_name == "" && type->TypeInfo.type_name == "") {
            return true;
        }

        if (type->_Node.Object().properties.count("_init_")) {
            type->_Node.Object().properties.erase("_init_");
        }

        if (value->_Node.Object().properties.count("_init_")) {
            value->_Node.Object().properties.erase("_init_");
        }

        if (type->_Node.Object().properties.size() != value->_Node.Object().properties.size()) {
            return false;
        }

        for (auto& prop : type->_Node.Object().properties) {
            std::string prop_name = prop.first;

            node_ptr a_prop_value = prop.second;
            node_ptr b_prop_value = value->_Node.Object().properties[prop_name];

            int match = match_types(a_prop_value, b_prop_value);
            if (!match) {
                return false;
            }
        }
        
        for (auto& prop : value->_Node.Object().properties) {
            std::string prop_name = prop.first;

            node_ptr b_prop_value = prop.second;
            node_ptr a_prop_value = value->_Node.Object().properties[prop_name];

            int match = match_types(b_prop_value, a_prop_value);
            if (!match) {
                return false;
            }
        }

        return true;
        
    }

    return true;
}

node_ptr Interpreter::eval_try_catch(node_ptr& node) {
    global_interpreter->try_catch = true;
    std::string error = "";
    for (node_ptr& expr : node->_Node.TryCatch().try_body->_Node.Object().elements) {
        node_ptr evaluated_expr = eval_node(expr);
        if (evaluated_expr->type == NodeType::ERROR) {
            error = evaluated_expr->_Node.Error().message;
            break;
        }
    }
    if (error != "") {
        global_interpreter->try_catch = false;
        sym_t_ptr catch_sym_table = std::make_shared<SymbolTable>();
        catch_sym_table->parent = current_symbol_table;
        node_ptr error_arg = node->_Node.TryCatch().catch_keyword->_Node.FunctionCall().args[0];
        node_ptr error_node = new_string_node(error);
        add_symbol(new_symbol(error_arg->_Node.ID().value, error_node), catch_sym_table);
        current_symbol_table = catch_sym_table;

        for (node_ptr& expr : node->_Node.TryCatch().catch_body->_Node.Object().elements) {
            eval_node(expr);
        }

        current_symbol_table = current_symbol_table->parent;
    }

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_object_init(node_ptr& node) {
    Symbol type_symbol = get_symbol(node->_Node.ObjectDeconstruct().name, current_symbol_table);
    if (type_symbol.value == nullptr) {
        return throw_error("Type '" + node->_Node.ObjectDeconstruct().name + "' is undefined");
    }
    node_ptr type = type_symbol.value;

    if (!type->TypeInfo.is_type) {
        return throw_error("Variable '" + node->_Node.ObjectDeconstruct().name + "' is not a type");
    }

    node_ptr object = new_node(NodeType::OBJECT);
    node_ptr init_props = eval_object(node->_Node.ObjectDeconstruct().body);
    object->TypeInfo.type = type_symbol.value;

    // Add defaults if they exist

    for (auto def : type->_Node.Object().defaults) {
        object->_Node.Object().properties[def.first] = std::make_shared<Node>(*def.second);
    }

    // Add Init props

    for (auto prop : init_props->_Node.Object().properties) {
        object->_Node.Object().properties[prop.first] = std::make_shared<Node>(*prop.second);
    }

    // Call the init function if it exists
    if (type->_Node.Object().properties.count("_init_")) {
        node_ptr func = type->_Node.Object().properties["_init_"];
        func->_Node.Function().closure["this"] = object;
        node_ptr func_call = new_node(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().name = "_init_";
        eval_func_call(func_call, func);
    }

    // Check that the value matches type
    int type_size = type->_Node.Object().properties.size();

    if (type->_Node.Object().properties.count("_init_")) {
        type_size--;
    }
    if (object->_Node.Object().properties.size() != type_size) {
        return throw_error("Error in object initialization for type '" + node->_Node.ObjectDeconstruct().name + "': Number of properties differs between object and type");
    }

    for (auto& prop : type->_Node.Object().properties) {
        if (prop.first == "_init_") {
            continue;
        }

        std::string prop_name = prop.first;

        node_ptr type_prop_value = prop.second;
        node_ptr obj_prop_value = object->_Node.Object().properties[prop_name];

        int match = match_types(type_prop_value, obj_prop_value);
        if (!match) {
            return throw_error("Error in object initialization for type '" + node->_Node.ObjectDeconstruct().name + "': Match error in property '" + prop_name + "'");
        }

        object->_Node.Object().properties[prop_name]->Hooks.onChange = type_prop_value->Hooks.onChange;
        object->_Node.Object().properties[prop_name]->Hooks.onCall = type_prop_value->Hooks.onCall;
        object->_Node.Object().properties[prop_name]->Meta = type_prop_value->Meta;
        object->_Node.Object().properties[prop_name]->TypeInfo.type = type_prop_value;
    }

    // We do a check in the other direction
    // To see if there are more props in object than what's defined

    for (auto& prop : object->_Node.Object().properties) {
        std::string prop_name = prop.first;

        node_ptr obj_prop_value = prop.second;
        node_ptr type_prop_value = type->_Node.Object().properties[prop_name];

        if (type_prop_value == nullptr) {
            return throw_error("Error in object initialization for type '" + node->_Node.ObjectDeconstruct().name + "': Match error in property '" + prop_name + "'");
        }
    }

    object->TypeInfo.type = type;
    object->TypeInfo.type_name = node->_Node.ObjectDeconstruct().name;
    return object;
}

node_ptr Interpreter::eval_load_lib(node_ptr& node) {
    auto args = node->_Node.FunctionCall().args;
    if (args.size() != 1) {
        return throw_error("Library loading expects 1 argument");
    }

    node_ptr path = eval_node(args[0]);

    if (path->type != NodeType::STRING) {
        return throw_error("Library loading expects 1 string argument");
    }

    node_ptr lib_node = new_node(NodeType::LIB);

    #if GCC_COMPILER
        #if __apple__ || __linux__
            void* handle = dlopen(path->_Node.String().value.c_str(), RTLD_LAZY);
            
            if (!handle) {
                return throw_error("Cannot open library: " + std::string(dlerror()));
            }

            typedef node_ptr (*call_function_t)(std::string name, std::vector<node_ptr> args);
            typedef void (*load_t)(Interpreter*& interpreter);

            dlerror();

            call_function_t call_function = (call_function_t) dlsym(handle, "call_function");
            const char* dlsym_error_call_function = dlerror();

            if (dlsym_error_call_function) {
                dlclose(handle);
                return throw_error("Error loading symbol 'call_function': " + std::string(dlsym_error_call_function));
            }

            load_t load = (load_t) dlsym(handle, "load");
            const char* dlsym_error_load = dlerror();

            if (!dlsym_error_load) {
                load(this);
            }

            lib_node->_Node.Lib().handle = handle;
            lib_node->_Node.Lib().call_function = call_function;

        #else

            typedef node_ptr (__cdecl *call_function_t)(std::string name, std::vector<node_ptr> args);
            typedef void (__cdecl *load_t)(Interpreter*& interpreter);

            HINSTANCE hinstLib; 
            call_function_t callFuncAddress;
            load_t loadFuncAddress;

            hinstLib = LoadLibrary(TEXT(path->_Node.String().value.c_str())); 

            if (hinstLib != NULL) { 
                callFuncAddress = (call_function_t) GetProcAddress(hinstLib, "call_function");

                if (callFuncAddress == NULL) {
                    return throw_error("Error finding function 'call_function'");
                }

                loadFuncAddress = (load_t) GetProcAddress(hinstLib, "load");

                if (loadFuncAddress != NULL) {
                    loadFuncAddress(this);
                }
            }

            lib_node->_Node.Lib().handle = hinstLib;
            lib_node->_Node.Lib().call_function = callFuncAddress;

        #endif
    #else
        #if __APPLE__ || __linux__
            void* handle = dlopen(path->_Node.String().value.c_str(), RTLD_LAZY);
            
            if (!handle) {
                return throw_error("Cannot open library: " + std::string(dlerror()));
            }

            typedef node_ptr (*call_function_t)(std::string name, std::vector<node_ptr> args);
            typedef void (*load_t)(Interpreter& interpreter);

            dlerror();

            call_function_t call_function = (call_function_t) dlsym(handle, "call_function");
            const char* dlsym_error_call_function = dlerror();

            if (dlsym_error_call_function) {
                dlclose(handle);
                return throw_error("Error loading symbol 'call_function': " + std::string(dlsym_error_call_function));
            }

            load_t load = (load_t) dlsym(handle, "load");
            const char* dlsym_error_load = dlerror();

            if (!dlsym_error_load) {
                load(*this->global_interpreter);
            }

            lib_node->_Node.Lib().handle = handle;
            lib_node->_Node.Lib().call_function = call_function;

        #else

            typedef node_ptr (__cdecl *call_function_t)(std::string name, std::vector<node_ptr> args);
            typedef void (__cdecl *load_t)(Interpreter*& interpretet);

            HINSTANCE hinstLib; 
            call_function_t callFuncAddress;
            load_t loadFuncAddress;

            hinstLib = LoadLibrary(TEXT(path->_Node.String().value.c_str())); 

            if (hinstLib != NULL) { 
                callFuncAddress = (call_function_t) GetProcAddress(hinstLib, "call_function");

                if (callFuncAddress == NULL) {
                    return throw_error("Error finding function 'call_function'");
                }

                loadFuncAddress = (load_t) GetProcAddress(hinstLib, "load");

                if (loadFuncAddress != NULL) {
                    loadFuncAddress(this);
                }
            }

            lib_node->_Node.Lib().handle = hinstLib;
            lib_node->_Node.Lib().call_function = callFuncAddress;

        #endif
    #endif

    return lib_node;
}

node_ptr Interpreter::eval_call_lib_function(node_ptr& lib, node_ptr& node) {
    auto args = node->_Node.FunctionCall().args;
    if (args.size() != 2) {
        return throw_error("Library function calls expects 2 arguments");
    }

    node_ptr name = eval_node(args[0]);
    node_ptr func_args = eval_node(args[1]);

    if (name->type != NodeType::STRING) {
        return throw_error("Library function calls expects first argument to be a string");
    }

    if (func_args->type != NodeType::LIST) {
        return throw_error("Library function calls expects first argument to be a list");
    }

    return lib->_Node.Lib().call_function(name->_Node.String().value, func_args->_Node.List().elements);
}

node_ptr Interpreter::eval_import(node_ptr& node) {

    if (node->_Node.Import().is_default) {
        node->_Node.Import().target = new_string_node("@modules/" + node->_Node.Import().module->_Node.ID().value);
    }

    if (node->_Node.Import().target->type == NodeType::ID) {
        node->_Node.Import().target = new_string_node("@modules/" + node->_Node.Import().target->_Node.ID().value);
    }

    if (node->_Node.Import().target->type != NodeType::STRING) {
        return throw_error("Import target must be a string");
    }

    std::string target_name = std::string(node->_Node.Import().target->_Node.String().value);
    replaceAll(target_name, "@modules/", "");

    std::string path = node->_Node.Import().target->_Node.String().value + ".vtx";

    #if GCC_COMPILER
        #if __apple__ || __linux__
            replaceAll(path, "@modules", "/usr/local/share/vortex/modules/" + target_name);
        #else
            replaceAll(path, "@modules", "C:/Program Files/Vortex/modules/" + target_name);
        #endif
    #else
        #if defined(__APPLE__) || defined(__linux__)
            replaceAll(path, "@modules", "/usr/local/share/vortex/modules/" + target_name);
        #else
            replaceAll(path, "@modules", "C:/Program Files/Vortex/modules/" + target_name);
        #endif
    #endif

    if (node->_Node.Import().module->type == NodeType::ID) {
        std::string module_name = node->_Node.Import().module->_Node.ID().value;
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
            if (parent_path != "") {
                std::filesystem::current_path(parent_path);
            }
        } catch(...) {
            return throw_error("No such file or directory: '" + parent_path.string() + "'");
        }

        Interpreter import_interpreter(import_parser.nodes, import_parser.file_name);
        import_interpreter.global_interpreter = this;
        import_interpreter.evaluate();

        std::filesystem::current_path(current_path);

        for (auto& symbol : import_interpreter.global_symbol_table->symbols) {
            import_obj->_Node.Object().properties[symbol.first] = symbol.second.value;
        }

        // We also want to import hooks into the current scope

        for (auto& hook : import_interpreter.global_symbol_table->globalHooks_onChange) {
            current_symbol_table->globalHooks_onChange.push_back(hook);
        }

        for (auto& hook : import_interpreter.global_symbol_table->globalHooks_onCall) {
            current_symbol_table->globalHooks_onCall.push_back(hook);
        }

        // Any Type extensions must be imported to global scope

        global_symbol_table->StringExtensions.insert(import_interpreter.global_symbol_table->StringExtensions.begin(), import_interpreter.global_symbol_table->StringExtensions.end());

        global_symbol_table->NumberExtensions.insert(import_interpreter.global_symbol_table->NumberExtensions.begin(), import_interpreter.global_symbol_table->NumberExtensions.end());

        global_symbol_table->BoolExtensions.insert(import_interpreter.global_symbol_table->BoolExtensions.begin(), import_interpreter.global_symbol_table->BoolExtensions.end());

        global_symbol_table->ListExtensions.insert(import_interpreter.global_symbol_table->ListExtensions.begin(), import_interpreter.global_symbol_table->ListExtensions.end());

        global_symbol_table->ObjectExtensions.insert(import_interpreter.global_symbol_table->ObjectExtensions.begin(), import_interpreter.global_symbol_table->ObjectExtensions.end());

        global_symbol_table->FunctionExtensions.insert(import_interpreter.global_symbol_table->FunctionExtensions.begin(), import_interpreter.global_symbol_table->FunctionExtensions.end());

        global_symbol_table->NoneExtensions.insert(import_interpreter.global_symbol_table->NoneExtensions.begin(), import_interpreter.global_symbol_table->NoneExtensions.end());

        global_symbol_table->CustomTypeExtensions.insert(import_interpreter.global_symbol_table->CustomTypeExtensions.begin(), import_interpreter.global_symbol_table->CustomTypeExtensions.end());

        add_symbol(new_symbol(module_name, import_obj), current_symbol_table);
        return new_node(NodeType::NONE);
    }

    if (node->_Node.Import().module->type == NodeType::LIST) {
        // Before we import, we'll check to see if the import list
        // contains only IDs
        if (node->_Node.Import().module->_Node.List().elements.size() == 1 && node->_Node.Import().module->_Node.List().elements[0]->type == NodeType::COMMA_LIST) {
            node->_Node.Import().module = node->_Node.Import().module->_Node.List().elements[0];
        }
        for (node_ptr elem : node->_Node.Import().module->_Node.List().elements) {
            if (elem->type != NodeType::ID) {
                return throw_error("Import list must contain identifiers");
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
            if (parent_path != "") {
                std::filesystem::current_path(parent_path);
            }
        } catch(...) {
            return throw_error("No such file or directory: '" + parent_path.string() + "'");
        }

        Interpreter import_interpreter(import_parser.nodes, import_parser.file_name);
        import_interpreter.global_interpreter = this;
        import_interpreter.evaluate();

        std::filesystem::current_path(current_path);

        std::unordered_map<std::string, node_ptr> imported_variables;

        for (auto& symbol : import_interpreter.global_symbol_table->symbols) {
            imported_variables[symbol.first] = symbol.second.value;
        }

        if (node->_Node.Import().module->_Node.List().elements.size() == 0) {
            for (auto& elem : import_interpreter.global_symbol_table->symbols) {
                add_symbol(new_symbol(elem.first, elem.second.value), current_symbol_table);
            }

            return new_node(NodeType::NONE);
        }

        for (node_ptr elem : node->_Node.Import().module->_Node.List().elements) {
            if (imported_variables.contains(elem->_Node.ID().value)) {
                add_symbol(import_interpreter.global_symbol_table->symbols[elem->_Node.ID().value], current_symbol_table);
            } else {
                return throw_error("Cannot import value '" + elem->_Node.ID().value + "' - variable undefined");
            }
        }

        return new_node(NodeType::NONE);
    }

    return throw_error("Malformed import statement");
    return new_node(NodeType::NONE);
}

// Operations

node_ptr Interpreter::eval_pos_neg(node_ptr& node) {
    node_ptr value = eval_node(node->_Node.Op().right);
    if (value->type != NodeType::NUMBER) {
        return throw_error("Cannot negate a non-number");
    }
    if (node->_Node.Op().value == "-") {
        return new_number_node(-value->_Node.Number().value);
    } else {
        return new_number_node(+value->_Node.Number().value);
    }

    return node;
}

node_ptr Interpreter::eval_not(node_ptr& node) {
    node_ptr value = eval_node(node->_Node.Op().right);

    if (value->type == NodeType::BOOLEAN) {
        return new_boolean_node(!value->_Node.Boolean().value);
    }

     return new_boolean_node(value->type == NodeType::NONE);
}

node_ptr Interpreter::eval_add(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->_Node.Number().value + right->_Node.Number().value);
    }

    if (left->type == NodeType::STRING && right->type == NodeType::STRING) {
        return new_string_node(left->_Node.String().value + right->_Node.String().value);
    }

    if (left->type == NodeType::LIST && right->type == NodeType::LIST) {
        node_ptr result = new_node(NodeType::LIST);
        result->_Node.List().elements.insert(result->_Node.List().elements.end(), left->_Node.List().elements.begin(), left->_Node.List().elements.end());
        result->_Node.List().elements.insert(result->_Node.List().elements.end(), right->_Node.List().elements.begin(), right->_Node.List().elements.end());
        return result;
    }

    return throw_error("Cannot perform operation '+' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_sub(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->_Node.Number().value - right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '-' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_mul(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->_Node.Number().value * right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '*' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_div(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->_Node.Number().value / right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '/' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_pow(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(pow(left->_Node.Number().value, right->_Node.Number().value));
    }

    return throw_error("Cannot perform operation '^' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_mod(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(fmod(left->_Node.Number().value, right->_Node.Number().value));
    }

    return throw_error("Cannot perform operation '^' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_eq_eq(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type != right->type) {
        return new_boolean_node(false);
    }

    if (left->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value == right->_Node.Number().value);
    }

    if (left->type == NodeType::STRING) {
        return new_boolean_node(left->_Node.String().value == right->_Node.String().value);
    }

    if (left->type == NodeType::BOOLEAN) {
        return new_boolean_node(left->_Node.Boolean().value == right->_Node.Boolean().value);
    }

    if (left->type == NodeType::NONE) {
        return new_boolean_node(true);
    }

    // TODO: Extend to lists and objects

    return new_boolean_node(false);
}

node_ptr Interpreter::eval_not_eq(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type != right->type) {
        return new_boolean_node(true);
    }

    if (left->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value != right->_Node.Number().value);
    }

    if (left->type == NodeType::STRING) {
        return new_boolean_node(left->_Node.String().value != right->_Node.String().value);
    }

    if (left->type == NodeType::BOOLEAN) {
        return new_boolean_node(left->_Node.Boolean().value != right->_Node.Boolean().value);
    }

    if (left->type == NodeType::NONE) {
        return new_boolean_node(false);
    }

    // TODO: Extend to lists and objects

    return new_boolean_node(true);
}

node_ptr Interpreter::eval_lt_eq(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value <= right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '<=' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_gt_eq(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value >= right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '>=' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_lt(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value < right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '<' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_gt(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value > right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '>' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_and(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);

    node_ptr left_bool = left;

    if (left->type != NodeType::BOOLEAN) {
        left_bool = new_boolean_node(left->type != NodeType::NONE);
    }

    if (!left_bool->_Node.Boolean().value) {
        return left;
    }

    node_ptr right = eval_node(node->_Node.Op().right);
    return right;

    // node_ptr right_bool = right;

    // if (right->type != NodeType::BOOLEAN) {
    //     right_bool = new_boolean_node(right->type != NodeType::NONE);
    // }

    // if (!right_bool->_Node.Boolean().value) {
    //     return right;
    // }

    // return left;

    // return throw_error("Cannot perform operation '&&' on types: " + node_repr(left) + ", " + node_repr(right));
    // return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_bit_and(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node((long)left->_Node.Number().value & (long)right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '&' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_bit_or(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node((long)left->_Node.Number().value | (long)right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '|' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_or(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr left_bool = left;

    if (left->type != NodeType::BOOLEAN) {
        left_bool = new_boolean_node(left->type != NodeType::NONE);
    }

    if (left_bool->_Node.Boolean().value) {
        return left;
    }

    node_ptr right = eval_node(node->_Node.Op().right);
    return right;
    // node_ptr right_bool = right;

    // if (right->type != NodeType::BOOLEAN) {
    //     right_bool = new_boolean_node(right->type != NodeType::NONE);
    // }

    // if (right_bool->type == NodeType::BOOLEAN) {
    //     return new_boolean_node(left->_Node.Boolean().value || right->_Node.Boolean().value);
    // }

    // return throw_error("Cannot perform operation '||' on types: " + node_repr(left) + ", " + node_repr(right));
    // return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_null_op(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);

    if (left->type != NodeType::NONE) {
        return left;
    }

    node_ptr right = eval_node(node->_Node.Op().right);
    return right;
}

node_ptr Interpreter::eval_plus_eq(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    node_ptr plus_node = new_node(NodeType::OP);
    plus_node->_Node.Op().value = "+";
    plus_node->_Node.Op().left = left;
    plus_node->_Node.Op().right = right;
    plus_node = eval_add(plus_node);

    node_ptr eq_node = new_node(NodeType::OP);
    eq_node->_Node.Op().value = "=";
    eq_node->_Node.Op().left = node->_Node.Op().left;
    eq_node->_Node.Op().right = plus_node;
    eq_node = eval_eq(eq_node);

    return eq_node;
}

node_ptr Interpreter::eval_minus_eq(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = eval_node(node->_Node.Op().right);

    node_ptr minus_node = new_node(NodeType::OP);
    minus_node->_Node.Op().value = "-";
    minus_node->_Node.Op().left = left;
    minus_node->_Node.Op().right = right;
    minus_node = eval_sub(minus_node);

    node_ptr eq_node = new_node(NodeType::OP);
    eq_node->_Node.Op().value = "=";
    eq_node->_Node.Op().left = node->_Node.Op().left;
    eq_node->_Node.Op().right = minus_node;
    eq_node = eval_eq(eq_node);

    return eq_node;
}

node_ptr Interpreter::eval_dot(node_ptr& node) {
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = node->_Node.Op().right;

    if (left->type == NodeType::OBJECT) {

        if (right->type == NodeType::ID) {
            if (left->_Node.Object().properties.contains(right->_Node.ID().value)) {
                return left->_Node.Object().properties[right->_Node.ID().value];
            }
            return new_node(NodeType::NONE);
        }

        if (right->type == NodeType::FUNC_CALL) {

            std::string func_name = right->_Node.FunctionCall().name;

            if (left->TypeInfo.type_name != "") {
                if (global_symbol_table->CustomTypeExtensions.count(left->TypeInfo.type_name)) {
                    if (global_symbol_table->CustomTypeExtensions[left->TypeInfo.type_name].count(func_name)) {
                        node_ptr ext = std::make_shared<Node>(*global_symbol_table->CustomTypeExtensions[left->TypeInfo.type_name][func_name]);
                        ext->_Node.Function().closure["self"] = left;
                        for (node_ptr& dispatch_fx : ext->_Node.Function().dispatch_functions) {
                            dispatch_fx->_Node.Function().closure["self"] = left;
                        }
                        return eval_func_call(right, ext);
                    }
                }
            }

            if (global_symbol_table->ObjectExtensions.count(func_name)) {
                node_ptr ext = std::make_shared<Node>(*global_symbol_table->ObjectExtensions[func_name]);
                ext->_Node.Function().closure["self"] = left;
                for (node_ptr& dispatch_fx : ext->_Node.Function().dispatch_functions) {
                    dispatch_fx->_Node.Function().closure["self"] = left;
                }
                return eval_func_call(right, ext);
            }

            if (left->_Node.Object().properties.count(func_name)) {
                right->_Node.FunctionCall().caller = left;
                return eval_func_call(right);
            }

            return throw_error("Method '" + func_name + "' does not exist on object");
        }

        if (right->type == NodeType::ACCESSOR) {
            node_ptr left_side = new_node(NodeType::OP);
            left_side->_Node.Op().value = ".";
            left_side->_Node.Op().left = left;
            left_side->_Node.Op().right = right->_Node.Accessor().container;
            left_side = eval_dot(left_side);

            node_ptr res = new_node(NodeType::ACCESSOR);
            res->_Node.Accessor().container = left_side;
            res->_Node.Accessor().accessor = right->_Node.Accessor().accessor;
            return eval_accessor(res);
        }

        return throw_error("Right hand side of '.' must be an identifier, function call or accessor");
    }

    if (left->type == NodeType::STRING) {
        // String Properties
        if (right->type == NodeType::ID) {
            std::string prop = right->_Node.ID().value;

            if (prop == "length") {
                return new_number_node(left->_Node.String().value.length());
            }
            if (prop == "empty") {
                return new_boolean_node(left->_Node.String().value.empty());
            }

            return throw_error("String does not have property '" + prop + "'");
        }

        if (right->type == NodeType::FUNC_CALL) {
            std::string func_name = right->_Node.FunctionCall().name;

            if (global_symbol_table->StringExtensions.count(func_name)) {
                node_ptr ext = std::make_shared<Node>(*global_symbol_table->StringExtensions[func_name]);
                ext->_Node.Function().closure["self"] = left;
                for (node_ptr& dispatch_fx : ext->_Node.Function().dispatch_functions) {
                    dispatch_fx->_Node.Function().closure["self"] = left;
                }
                return eval_func_call(right, ext);
            }

            return throw_error("String does not have method '" + func_name + "'");
        }

        return throw_error("Cannot perform '.' on types: " + node_repr(left) + ", " + node_repr(right));
    }

    if (left->type == NodeType::LIST) {

        node_ptr list_type = left->TypeInfo.type;
        if (!list_type) {
            list_type = new_node(NodeType::ANY);
        }else if (list_type->_Node.List().elements.size() == 1) {
            list_type = list_type->_Node.List().elements[0];
        } else {
            list_type = new_node(NodeType::ANY);
        }

        // List Properties
        if (right->type == NodeType::ID) {
            std::string prop = right->_Node.ID().value;

            if (prop == "length") {
                return new_number_node(left->_Node.List().elements.size());
            }
            if (prop == "empty") {
                return new_boolean_node(left->_Node.List().elements.empty());
            }

            return throw_error("List does not have property '" + prop + "'");
        }

        if (right->type == NodeType::FUNC_CALL) {
            std::string prop = right->_Node.FunctionCall().name;

            if (prop == "append") {
                if (left->Meta.is_const) {
                    return throw_error("Cannot modify constant list");
                }
                if (right->_Node.FunctionCall().args.size() != 1) {
                    return throw_error("List function '" + prop + "' expects 1 argument");
                }
                node_ptr arg = eval_node(right->_Node.FunctionCall().args[0]);
                // Type check
                if (!match_types(list_type, arg)) {
                    return throw_error("Cannot insert value of type '" + node_repr(arg) + "' into container of type '" + node_repr(left->TypeInfo.type) + "'");
                }
                left->_Node.List().elements.push_back(arg);
                return left;
            }
            if (prop == "prepend") {
                if (left->Meta.is_const) {
                    return throw_error("Cannot modify constant list");
                }
                if (right->_Node.FunctionCall().args.size() != 1) {
                    return throw_error("List function '" + prop + "' expects 1 argument");
                }
                node_ptr arg = eval_node(right->_Node.FunctionCall().args[0]);
                // Type check
                if (!match_types(list_type, arg)) {
                    return throw_error("Cannot insert value of type '" + node_repr(arg) + "' into container of type '" + node_repr(left->TypeInfo.type) + "'");
                }
                left->_Node.List().elements.insert(left->_Node.List().elements.begin(), arg);
                return left;
            }
            if (prop == "insert") {
                if (left->Meta.is_const) {
                    return throw_error("Cannot modify constant list");
                }
                if (right->_Node.FunctionCall().args.size() != 2) {
                    return throw_error("List function '" + prop + "' expects 2 arguments");
                }
                node_ptr value = eval_node(right->_Node.FunctionCall().args[0]);
                node_ptr index_node = eval_node(right->_Node.FunctionCall().args[1]);

                // Type check
                if (!match_types(list_type, value)) {
                    return throw_error("Cannot insert value of type '" + node_repr(value) + "' into container of type '" + node_repr(left->TypeInfo.type) + "'");
                }

                if (index_node->type != NodeType::NUMBER) {
                    return throw_error("List function '" + prop + "' expects second argument to be a number");
                }

                int index = index_node->_Node.Number().value;

                if (index < 0) {
                    index = 0;
                } else if (index > left->_Node.List().elements.size()) {
                    index = left->_Node.List().elements.size();
                }

                left->_Node.List().elements.insert(left->_Node.List().elements.begin() + index, value);
                return left;
            }
            if (prop == "remove_at") {
                if (left->Meta.is_const) {
                    return throw_error("Cannot modify constant list");
                }
                if (left->_Node.List().elements.size() == 0) {
                    return left;
                }

                if (right->_Node.FunctionCall().args.size() != 1) {
                    return throw_error("List function '" + prop + "' expects 1 argument");
                }

                node_ptr index_node = eval_node(right->_Node.FunctionCall().args[0]);

                if (index_node->type != NodeType::NUMBER) {
                    return throw_error("List function '" + prop + "' expects argument to be a number");
                }

                int index = index_node->_Node.Number().value;

                if (index < 0) {
                    index = 0;
                } else if (index > left->_Node.List().elements.size()-1) {
                    index = left->_Node.List().elements.size()-1;
                }

                left->_Node.List().elements.erase(left->_Node.List().elements.begin() + index);
                return left;
            }
            if (prop == "remove") {
                if (left->Meta.is_const) {
                    return throw_error("Cannot modify constant list");
                }
                if (right->_Node.FunctionCall().args.size() != 1) {
                    return throw_error("List function '" + prop + "' expects 1 argument");
                }
                node_ptr value = eval_node(right->_Node.FunctionCall().args[0]);

                for (int _index = 0; _index < left->_Node.List().elements.size(); _index++) {
                    node_ptr eq_eq = new_node(NodeType::OP);
                    eq_eq->_Node.Op().value = "==";
                    eq_eq->_Node.Op().left = left->_Node.List().elements[_index];
                    eq_eq->_Node.Op().right = value;
                    eq_eq = eval_eq_eq(eq_eq);
                    if (eq_eq->_Node.Boolean().value) {
                        left->_Node.List().elements.erase(left->_Node.List().elements.begin() + _index);
                        _index--;
                    }
                }

                return left;
            }
            if (prop == "remove_if") {
                if (left->Meta.is_const) {
                    return throw_error("Cannot modify constant list");
                }
                if (right->_Node.FunctionCall().args.size() != 1) {
                    return throw_error("List function '" + prop + "' expects 1 argument");
                }
                
                node_ptr function = right->_Node.FunctionCall().args[0];

                if (function->type != NodeType::FUNC) {
                    return throw_error("List function '" + prop + "' expects 1 function argument");
                }

                for (int _index = 0; _index < left->_Node.List().elements.size(); _index++) {
                    node_ptr value = left->_Node.List().elements[_index];
                    node_ptr func_call = new_node(NodeType::FUNC_CALL);
                    if (function->_Node.Function().params.size() == 0) {
                        return throw_error("Function needs to have at least one parameter");
                    }
                    if (function->_Node.Function().params.size() > 0) {
                        func_call->_Node.FunctionCall().args.push_back(value);
                    }
                    if (function->_Node.Function().params.size() > 1) {
                        func_call->_Node.FunctionCall().args.push_back(new_number_node(_index));
                    }
                    if (function->_Node.Function().params.size() > 2) {
                        func_call->_Node.FunctionCall().args.push_back(left);
                    }
                    node_ptr res = eval_func_call(func_call, function);
                    if (res->type != NodeType::BOOLEAN) {
                        return throw_error("Function must return a boolean value");
                    }
                    if (res->_Node.Boolean().value) {
                        left->_Node.List().elements.erase(left->_Node.List().elements.begin() + _index);
                        _index--;
                    }
                }

                return left;
            }

            // Functional Operations

            if (prop == "map") {
                if (right->_Node.FunctionCall().args.size() != 1) {
                    return throw_error("List function '" + prop + "' expects 1 argument");
                }
                
                node_ptr function = eval_node(right->_Node.FunctionCall().args[0]);

                if (function->type != NodeType::FUNC) {
                    return throw_error("List function '" + prop + "' expects 1 function argument");
                }

                node_ptr new_list = new_node(NodeType::LIST);

                for (int _index = 0; _index < left->_Node.List().elements.size(); _index++) {
                    node_ptr value = left->_Node.List().elements[_index];
                    node_ptr func_call = new_node(NodeType::FUNC_CALL);
                    if (function->_Node.Function().params.size() == 0) {
                        return throw_error("Function needs to have at least one parameter");
                    }
                    if (function->_Node.Function().params.size() > 0) {
                        func_call->_Node.FunctionCall().args.push_back(value);
                    }
                    if (function->_Node.Function().params.size() > 1) {
                        func_call->_Node.FunctionCall().args.push_back(new_number_node(_index));
                    }
                    if (function->_Node.Function().params.size() > 2) {
                        func_call->_Node.FunctionCall().args.push_back(left);
                    }
                    node_ptr res = eval_func_call(func_call, function);

                    new_list->_Node.List().elements.push_back(res);
                }

                return new_list;
            }

            if (prop == "filter") {
                if (right->_Node.FunctionCall().args.size() != 1) {
                    return throw_error("List function '" + prop + "' expects 1 argument");
                }
                
                node_ptr function = eval_node(right->_Node.FunctionCall().args[0]);

                if (function->type != NodeType::FUNC) {
                    return throw_error("List function '" + prop + "' expects 1 function argument");
                }

                node_ptr new_list = new_node(NodeType::LIST);

                for (int _index = 0; _index < left->_Node.List().elements.size(); _index++) {
                    node_ptr value = left->_Node.List().elements[_index];
                    node_ptr func_call = new_node(NodeType::FUNC_CALL);
                    if (function->_Node.Function().params.size() == 0) {
                        return throw_error("Function needs to have at least one parameter");
                    }
                    if (function->_Node.Function().params.size() > 0) {
                        func_call->_Node.FunctionCall().args.push_back(value);
                    }
                    if (function->_Node.Function().params.size() > 1) {
                        func_call->_Node.FunctionCall().args.push_back(new_number_node(_index));
                    }
                    if (function->_Node.Function().params.size() > 2) {
                        func_call->_Node.FunctionCall().args.push_back(left);
                    }
                    node_ptr res = eval_func_call(func_call, function);

                    if (res->type != NodeType::BOOLEAN) {
                        return throw_error("Function must return a boolean value");
                    }

                    if (res->_Node.Boolean().value) {
                        new_list->_Node.List().elements.push_back(value);
                    }
                }

                return new_list;
            }

            if (prop == "reduce") {
                if (right->_Node.FunctionCall().args.size() != 1) {
                    return throw_error("List function '" + prop + "' expects 1 argument");
                }
                
                node_ptr function = eval_node(right->_Node.FunctionCall().args[0]);

                if (function->type != NodeType::FUNC) {
                    return throw_error("List function '" + prop + "' expects 1 function argument");
                }

                node_ptr new_list = new_node(NodeType::LIST);

                if (left->_Node.List().elements.size() < 2) {
                    return left;
                }

                for (int _index = 0; _index < left->_Node.List().elements.size()-1; _index++) {
                    node_ptr func_call = new_node(NodeType::FUNC_CALL);
                    if (function->_Node.Function().params.size() < 2) {
                        return throw_error("Function needs to have at least two parameters");
                    }

                    node_ptr value = left->_Node.List().elements[_index];
                    node_ptr next_value = left->_Node.List().elements[_index + 1];

                    if (new_list->_Node.List().elements.size() > 0) {
                        value = new_list->_Node.List().elements[0];
                    }

                    func_call->_Node.FunctionCall().args.push_back(value);
                    func_call->_Node.FunctionCall().args.push_back(next_value);

                    if (function->_Node.Function().params.size() > 2) {
                        func_call->_Node.FunctionCall().args.push_back(new_number_node(_index));
                    }
                    if (function->_Node.Function().params.size() > 3) {
                        func_call->_Node.FunctionCall().args.push_back(left);
                    }

                    node_ptr res = eval_func_call(func_call, function);
                    value = res;

                    if (new_list->_Node.List().elements.size() == 0) {
                        new_list->_Node.List().elements.push_back(res);
                    } else {
                        new_list->_Node.List().elements[0] = res;
                    }
                }

                return new_list->_Node.List().elements[0];
            }

            std::string func_name = right->_Node.FunctionCall().name;

            if (global_symbol_table->ListExtensions.count(func_name)) {
                node_ptr ext = std::make_shared<Node>(*global_symbol_table->ListExtensions[func_name]);
                ext->_Node.Function().closure["self"] = left;
                for (node_ptr& dispatch_fx : ext->_Node.Function().dispatch_functions) {
                    dispatch_fx->_Node.Function().closure["self"] = left;
                }
                return eval_func_call(right, ext);
            }

            return throw_error("List does not have method '" + func_name + "'");
        }

        return throw_error("Right hand side of '.' must be an identifier or function call");
    }

    if (left->type == NodeType::HOOK) {
        std::string hook_name = left->_Node.Hook().hook_name;
        std::string name = left->_Node.Hook().name;
        node_ptr hook_func = eval_node(left->_Node.Hook().function);
        hook_func->_Node.Function().name = name;
        hook_func->_Node.Function().is_hook = true;

        if (right->type == NodeType::FUNC_CALL) {
            std::string func_name = right->_Node.FunctionCall().name;

            // TODO: Change hooks to be map
            // Leaving as list due to current implementations

            if (func_name == "attach") {
                if (right->_Node.FunctionCall().args.size() == 0) {
                    if (hook_name == "onChange") {
                        global_symbol_table->globalHooks_onChange.push_back(hook_func);
                    }
                    if (hook_name == "onCall") {
                        global_symbol_table->globalHooks_onCall.push_back(hook_func);
                    }
                    return new_node(NodeType::NONE);
                }
                if (right->_Node.FunctionCall().args.size() == 1) {
                    node_ptr arg = eval_node(right->_Node.FunctionCall().args[0]);

                    if (hook_name == "onChange") {
                        if (arg->type == NodeType::LIST) {
                            for (auto elem : arg->_Node.List().elements) {
                                elem->Hooks.onChange.push_back(hook_func);
                            }
                        } else {
                            arg->Hooks.onChange.push_back(hook_func);
                        }
                    }
                    if (hook_name == "onCall") {
                        if (arg->type == NodeType::LIST) {
                            for (auto elem : arg->_Node.List().elements) {
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
                if (right->_Node.FunctionCall().args.size() == 0) {
                    if (hook_name == "onChange") {
                        for (int i = 0; i < global_symbol_table->globalHooks_onChange.size(); i++) {
                            node_ptr func = global_symbol_table->globalHooks_onChange[i];
                            if (func->_Node.Function().name == name) {
                                global_symbol_table->globalHooks_onChange.erase(global_symbol_table->globalHooks_onChange.begin() + i);
                            }
                        }
                    }
                    if (hook_name == "onCall") {
                        for (int i = 0; i < global_symbol_table->globalHooks_onCall.size(); i++) {
                            node_ptr func = global_symbol_table->globalHooks_onCall[i];
                            if (func->_Node.Function().name == name) {
                                global_symbol_table->globalHooks_onCall.erase(global_symbol_table->globalHooks_onCall.begin() + i);
                            }
                        }
                    }
                    return new_node(NodeType::NONE);
                }
                if (right->_Node.FunctionCall().args.size() == 1) {
                    node_ptr arg = eval_node(right->_Node.FunctionCall().args[0]);

                    if (hook_name == "onChange") {
                        if (arg->type == NodeType::LIST) {
                            for (auto elem : arg->_Node.List().elements) {
                                for (int i = 0; i < elem->Hooks.onChange.size(); i++) {
                                    if (elem->Hooks.onChange[i]->_Node.Function().name == name) {
                                        elem->Hooks.onChange.erase(elem->Hooks.onChange.begin() + i);
                                    }
                                }
                            }
                        } else {
                            for (int i = 0; i < arg->Hooks.onChange.size(); i++) {
                                if (arg->Hooks.onChange[i]->_Node.Function().name == name) {
                                    arg->Hooks.onChange.erase(arg->Hooks.onChange.begin() + i);
                                }
                            }
                        }
                    }
                    if (hook_name == "onCall") {
                        if (arg->type == NodeType::LIST) {
                            for (auto elem : arg->_Node.List().elements) {
                                for (int i = 0; i < elem->Hooks.onCall.size(); i++) {
                                    if (elem->Hooks.onCall[i]->_Node.Function().name == name) {
                                        elem->Hooks.onCall.erase(elem->Hooks.onCall.begin() + i);
                                    }
                                }
                            }
                        } else {
                            for (int i = 0; i < arg->Hooks.onCall.size(); i++) {
                                if (arg->Hooks.onCall[i]->_Node.Function().name == name) {
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
            if (right->_Node.FunctionCall().name == "call") {
                return eval_call_lib_function(left, right);
            }
        }

        return throw_error("Cannot perform '.' on types: " + node_repr(left) + ", " + node_repr(right));
    }

    if (left->type == NodeType::NUMBER) {
        if (right->type == NodeType::FUNC_CALL) {
            std::string func_name = right->_Node.FunctionCall().name;

            if (global_symbol_table->NumberExtensions.count(func_name)) {
                node_ptr ext = std::make_shared<Node>(*global_symbol_table->NumberExtensions[func_name]);
                ext->_Node.Function().closure["self"] = left;
                for (node_ptr& dispatch_fx : ext->_Node.Function().dispatch_functions) {
                    dispatch_fx->_Node.Function().closure["self"] = left;
                }
                return eval_func_call(right, ext);
            }

            return throw_error("Number does not have method '" + func_name + "'");
        }
    }

    if (left->type == NodeType::BOOLEAN) {
        if (right->type == NodeType::FUNC_CALL) {
            std::string func_name = right->_Node.FunctionCall().name;

            if (global_symbol_table->BoolExtensions.count(func_name)) {
                node_ptr ext = std::make_shared<Node>(*global_symbol_table->BoolExtensions[func_name]);
                ext->_Node.Function().closure["self"] = left;
                for (node_ptr& dispatch_fx : ext->_Node.Function().dispatch_functions) {
                    dispatch_fx->_Node.Function().closure["self"] = left;
                }
                return eval_func_call(right, ext);
            }

            return throw_error("Boolean does not have method '" + func_name + "'");
        }
    }

    if (left->type == NodeType::FUNC) {
        if (right->type == NodeType::FUNC_CALL) {
            std::string func_name = right->_Node.FunctionCall().name;

            if (global_symbol_table->FunctionExtensions.count(func_name)) {
                node_ptr ext = std::make_shared<Node>(*global_symbol_table->FunctionExtensions[func_name]);
                ext->_Node.Function().closure["self"] = left;
                for (node_ptr& dispatch_fx : ext->_Node.Function().dispatch_functions) {
                    dispatch_fx->_Node.Function().closure["self"] = left;
                }
                return eval_func_call(right, ext);
            }

            return throw_error("Function does not have method '" + func_name + "'");
        }
    }

    if (left->type == NodeType::NONE) {
        if (right->type == NodeType::FUNC_CALL) {
            std::string func_name = right->_Node.FunctionCall().name;

            if (global_symbol_table->NoneExtensions.count(func_name)) {
                node_ptr ext = std::make_shared<Node>(*global_symbol_table->NoneExtensions[func_name]);
                ext->_Node.Function().closure["self"] = left;
                for (node_ptr& dispatch_fx : ext->_Node.Function().dispatch_functions) {
                    dispatch_fx->_Node.Function().closure["self"] = left;
                }
                return eval_func_call(right, ext);
            }

            return throw_error("None does not have method '" + func_name + "'");
        }
    }

    return throw_error("Cannot perform '.' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Interpreter::eval_eq(node_ptr& node) {
    node_ptr left = node->_Node.Op().left;
    node_ptr right = eval_node(node->_Node.Op().right);

    if (left->type == NodeType::OP && left->_Node.Op().value == ".") {
        node_ptr object = eval_node(left->_Node.Op().left);
        if (object->Meta.is_const) {
            return throw_error("Cannot modify constant object");
        }
        node_ptr prop = left->_Node.Op().right;

        if (object->type != NodeType::OBJECT) {
            return throw_error("Left hand side of '.' must be an object");
        }

        if (prop->type != NodeType::ID && prop->type != NodeType::ACCESSOR) {
            return throw_error("Right hand side of '.' must be an identifier");
        }

        if (prop->type == NodeType::ACCESSOR) {
            node_ptr accessed_value = eval_dot(left);
            node_ptr old_value = std::make_shared<Node>(*accessed_value);
            std::vector<node_ptr> onChangeFunctions = accessed_value->Hooks.onChange;

            // Type check
            // if (!accessed_value->Meta.is_untyped_property) {
            //     if (accessed_value->type != NodeType::NONE && !match_types(accessed_value->TypeInfo.type, right)) {
            //         return throw_error("Cannot modify object property type '" + node_repr(accessed_value) + "'");
            //     }
            // }
            if (!accessed_value->Meta.is_untyped_property) {
                if (!match_types(accessed_value, right)) {
                    return throw_error("Cannot modify object property type '" + node_repr(accessed_value) + "'");
                }
            }

            if (accessed_value->type == NodeType::NONE && prop->_Node.Accessor().container->TypeInfo.type) {
                return throw_error("Property '" + prop->_Node.ID().value + "' does not exist on type '" + object->TypeInfo.type_name + "'");
            }

            *accessed_value = *right;
            accessed_value->TypeInfo = old_value->TypeInfo;
            accessed_value->Meta = old_value->Meta;
            accessed_value->Meta.is_const = false;
            accessed_value->Hooks.onChange = onChangeFunctions;

            auto allOnChangeFunctionsLists = {std::cref(onChangeFunctions), std::cref(global_symbol_table->globalHooks_onChange)};
            
            for (const auto& function_list : allOnChangeFunctionsLists) {
                for (auto function : function_list.get()) {
                    node_ptr function_call = new_node(NodeType::FUNC_CALL);
                    function_call->_Node.FunctionCall().name = function->_Node.Function().name;
                    function_call->_Node.FunctionCall().args = std::vector<node_ptr>();
                    if (function->_Node.Function().params.size() > 0) {
                        function_call->_Node.FunctionCall().args.push_back(accessed_value);
                    }
                    if (function->_Node.Function().params.size() > 1) {
                        function_call->_Node.FunctionCall().args.push_back(old_value);
                    }
                    if (function->_Node.Function().params.size() > 2) {
                        node_ptr file_info = new_node(NodeType::OBJECT);
                        file_info->_Node.Object().properties["name"] = new_string_node(printable(left->_Node.Op().left) + "." + printable(prop));
                        file_info->_Node.Object().properties["filename"] = new_string_node(file_name);
                        file_info->_Node.Object().properties["line"] = new_number_node(line);
                        file_info->_Node.Object().properties["column"] = new_number_node(column);
                        function_call->_Node.FunctionCall().args.push_back(file_info);
                    }
                    eval_func_call(function_call, function);
                }
            }

            return object;
        }

        node_ptr accessed_value = object->_Node.Object().properties[prop->_Node.ID().value];
        if (!accessed_value) {
            // If the object has a type and the property wasn't found, we error
            if (object->TypeInfo.type) {
                return throw_error("Property '" + prop->_Node.ID().value + "' does not exist on type '" + object->TypeInfo.type_name + "'");
            }   
            accessed_value = new_node(NodeType::NONE);
        }
        node_ptr old_value = std::make_shared<Node>(*accessed_value);
        std::vector<node_ptr> onChangeFunctions = old_value->Hooks.onChange;

        // Type check
        // if (!accessed_value->Meta.is_untyped_property) {
        //     if (accessed_value->type != NodeType::NONE && !match_types(accessed_value->TypeInfo.type, right)) {
        //         return throw_error("Type error in property '" + prop->_Node.ID().value + "' - Cannot modify object property type '" + node_repr(accessed_value) + "'");
        //     }
        // }
        if (!accessed_value->Meta.is_untyped_property) {
            if (!match_types(accessed_value, right)) {
                return throw_error("Type error in property '" + prop->_Node.ID().value + "' - Cannot modify object property type '" + node_repr(accessed_value) + "'");
            }
        }

        *accessed_value = *right;
        accessed_value->Meta = old_value->Meta;
        accessed_value->TypeInfo = old_value->TypeInfo;
        accessed_value->Hooks.onChange = onChangeFunctions;

        auto allOnChangeFunctionsLists = {std::cref(onChangeFunctions), std::cref(global_symbol_table->globalHooks_onChange)};
            
        for (const auto& function_list : allOnChangeFunctionsLists) {
            for (node_ptr function : function_list.get()) {
                node_ptr function_call = new_node(NodeType::FUNC_CALL);
                function_call->_Node.FunctionCall().name = function->_Node.Function().name;
                function_call->_Node.FunctionCall().args = std::vector<node_ptr>();
                if (function->_Node.Function().params.size() > 0) {
                    function_call->_Node.FunctionCall().args.push_back(accessed_value);
                }
                if (function->_Node.Function().params.size() > 1) {
                    function_call->_Node.FunctionCall().args.push_back(old_value);
                }
                if (function->_Node.Function().params.size() > 2) {
                    node_ptr file_info = new_node(NodeType::OBJECT);
                    file_info->_Node.Object().properties["name"] = new_string_node(printable(left->_Node.Op().left) + "." + printable(prop));
                    file_info->_Node.Object().properties["filename"] = new_string_node(file_name);
                    file_info->_Node.Object().properties["line"] = new_number_node(line);
                    file_info->_Node.Object().properties["column"] = new_number_node(column);
                    function_call->_Node.FunctionCall().args.push_back(file_info);
                }
                eval_func_call(function_call, function);
            }
        }

        return object;
    }

    if (left->type == NodeType::ACCESSOR) {
        node_ptr container = eval_node(left->_Node.Accessor().container);
        node_ptr accessor = eval_node(left->_Node.Accessor().accessor);
        if (container->Meta.is_const) {
            return throw_error("Cannot modify constant");
        }
        if (container->type == NodeType::LIST) {
            accessor = eval_node(accessor->_Node.List().elements[0]);
            if (accessor->type != NodeType::NUMBER) {
                return throw_error("List accessor must be a number");
            }

            node_ptr accessed_value = eval_accessor(left);

            if (accessor->_Node.Number().value < 0) {
                container->_Node.List().elements.insert(container->_Node.List().elements.begin(), right);
            } else if (accessor->_Node.Number().value >= container->_Node.List().elements.size()) {
                container->_Node.List().elements.push_back(right);
            } else {
                container->_Node.List().elements[accessor->_Node.Number().value] = right;
            }
        } else if (container->type == NodeType::OBJECT) {
            node_ptr accessed_value = eval_accessor(left);

            // If the object has a type and the property wasn't found, we error
            if (container->TypeInfo.type_name != "" && accessed_value->type == NodeType::NONE) {
                return throw_error("Property does not exist on type '" + container->TypeInfo.type_name + "'");
            }  

            if (accessed_value->type == NodeType::NONE) {
                container->_Node.Object().properties[accessor->_Node.List().elements[0]->_Node.String().value] = right;
            } else {
                node_ptr old_value = std::make_shared<Node>(*accessed_value);
                std::vector<node_ptr> onChangeFunctions = accessed_value->Hooks.onChange;

                // Type check
                // if (!accessed_value->Meta.is_untyped_property) {
                //     if (accessed_value->type != NodeType::NONE && !match_types(accessed_value->TypeInfo.type, right)) {
                //         return throw_error("Cannot modify object property type '" + node_repr(accessed_value) + "'");
                //     }
                // }
                if (!accessed_value->Meta.is_untyped_property) {
                    if (!match_types(accessed_value, right)) {
                        return throw_error("Cannot modify object property type '" + node_repr(accessed_value) + "'");
                    }
                }

                *accessed_value = *right;
                accessed_value->TypeInfo = old_value->TypeInfo;
                accessed_value->Meta = old_value->Meta;
                accessed_value->Meta.is_const = false;
                accessed_value->Hooks.onChange = onChangeFunctions;

                auto allOnChangeFunctionsLists = {std::cref(onChangeFunctions), std::cref(global_symbol_table->globalHooks_onChange)};
                
                for (const auto& function_list : allOnChangeFunctionsLists) {
                    for (auto function : function_list.get()) {
                        node_ptr function_call = new_node(NodeType::FUNC_CALL);
                        function_call->_Node.FunctionCall().name = function->_Node.Function().name;
                        function_call->_Node.FunctionCall().args = std::vector<node_ptr>();
                        if (function->_Node.Function().params.size() > 0) {
                            function_call->_Node.FunctionCall().args.push_back(accessed_value);
                        }
                        if (function->_Node.Function().params.size() > 1) {
                            function_call->_Node.FunctionCall().args.push_back(old_value);
                        }
                        if (function->_Node.Function().params.size() > 2) {
                            node_ptr file_info = new_node(NodeType::OBJECT);
                            file_info->_Node.Object().properties["name"] = new_string_node(printable(left));
                            file_info->_Node.Object().properties["filename"] = new_string_node(file_name);
                            file_info->_Node.Object().properties["line"] = new_number_node(line);
                            file_info->_Node.Object().properties["column"] = new_number_node(column);
                            function_call->_Node.FunctionCall().args.push_back(file_info);
                        }
                        eval_func_call(function_call, function);
                    }
                }
            }
        }
        return right;
    }

    if (left->type == NodeType::ID) {
        Symbol symbol = get_symbol(left->_Node.ID().value, current_symbol_table);
        if (symbol.value == nullptr) {
            return throw_error("Variable '" + left->_Node.ID().value + "' is undefined");
        } else {
            // Re-assigning, check if const
            if (symbol.value->Meta.is_const) {
                return throw_error("Cannot modify constant '" + symbol.name + "'");
            }

            if (!match_types(symbol.value, right)) {
                node_ptr type = symbol.value;
                if (!type->TypeInfo.is_type && type->TypeInfo.type) {
                    type = type->TypeInfo.type;
                }
                return throw_error("Cannot modify type of variable '" + symbol.name + "': Expected '" + node_repr(type) + "' but received '" + node_repr(right) + "'");
            }

            node_ptr old_value = std::make_shared<Node>(*symbol.value);

            // Extract onChange functions from value

            std::vector<node_ptr> onChangeFunctions = symbol.value->Hooks.onChange;

            *symbol.value = *right;
            symbol.value->TypeInfo = old_value->TypeInfo;
            symbol.value->Meta.is_const = false;
            symbol.value->Hooks.onChange = onChangeFunctions;

            // Call onChange functions

            auto allOnChangeFunctionsLists = {std::cref(onChangeFunctions), std::cref(global_symbol_table->globalHooks_onChange)};
            
            for (const auto& function_list : allOnChangeFunctionsLists) {
                for (node_ptr function : function_list.get()) {
                    node_ptr function_call = new_node(NodeType::FUNC_CALL);
                    function_call->_Node.FunctionCall().name = function->_Node.Function().name;
                    function_call->_Node.FunctionCall().args = std::vector<node_ptr>();
                    if (function->_Node.Function().params.size() > 0) {
                        function_call->_Node.FunctionCall().args.push_back(symbol.value);
                    }
                    if (function->_Node.Function().params.size() > 1) {
                        function_call->_Node.FunctionCall().args.push_back(old_value);
                    }
                    if (function->_Node.Function().params.size() > 2) {
                        node_ptr file_info = new_node(NodeType::OBJECT);
                        file_info->_Node.Object().properties["name"] = new_string_node(symbol.name);
                        file_info->_Node.Object().properties["filename"] = new_string_node(file_name);
                        file_info->_Node.Object().properties["line"] = new_number_node(line);
                        file_info->_Node.Object().properties["column"] = new_number_node(column);
                        function_call->_Node.FunctionCall().args.push_back(file_info);
                    }
                    eval_func_call(function_call, function);
                }
            }

            return right;
        }

        return throw_error("Variable '" + left->_Node.ID().value + "' is undefined");
    }

    if (left->_Node.Op().value == "::") {
        node_ptr target = left->_Node.Op().left;
        node_ptr prop = left->_Node.Op().right;

        right->_Node.Function().is_hook = true;

        if (prop->type != NodeType::ID) {
            return throw_error("Hook expects an identifier");
        }

        if (target->type == NodeType::ID) {
            Symbol symbol = get_symbol(target->_Node.ID().value, current_symbol_table);
            if (symbol.value == nullptr) {
                return throw_error("Variable '" + target->_Node.ID().value + "' is undefined");
            }

            if (prop->_Node.ID().value == "onChange") {
                if (right->type != NodeType::FUNC) {
                    return throw_error("onChange hook expects a function");
                }
                symbol.value->Hooks.onChange.push_back(right);
            } else if (prop->_Node.ID().value == "onCall") {
                if (right->type != NodeType::FUNC && symbol.value->type != NodeType::FUNC) {
                    return throw_error("onCall hook expects a function");
                }
                symbol.value->Hooks.onCall.push_back(right);
            } else {
                return throw_error("Unknown hook '" + prop->_Node.ID().value + "'");
            }

            add_symbol(symbol, current_symbol_table);

            return new_node(NodeType::NONE);
        }

        if (target->type == NodeType::LIST) {
            // Global hook
            if (target->_Node.List().elements.size() == 0) {
                if (prop->_Node.ID().value == "onChange") {
                    if (right->type != NodeType::FUNC) {
                        return throw_error("onChange hook expects a function");
                    }
                    global_symbol_table->globalHooks_onChange.push_back(right);
                } else if (prop->_Node.ID().value == "onCall") {
                    if (right->type != NodeType::FUNC) {
                        return throw_error("onCall hook expects a function");
                    }
                    global_symbol_table->globalHooks_onCall.push_back(right);
                } else {
                    return throw_error("Unknown hook '" + prop->_Node.ID().value + "'");
                }

                return new_node(NodeType::NONE);
            }

            if (target->_Node.List().elements.size() == 1 && target->_Node.List().elements[0]->type == NodeType::COMMA_LIST) {
                target = target->_Node.List().elements[0];
            }

            for (node_ptr elem : target->_Node.List().elements) {
                elem = eval_node(elem);

                if (prop->_Node.ID().value == "onChange") {
                    if (right->type != NodeType::FUNC) {
                        return throw_error("onChange hook expects a function");
                    }
                    elem->Hooks.onChange.push_back(right);
                } else if (prop->_Node.ID().value == "onCall") {
                    if (right->type != NodeType::FUNC && elem->type != NodeType::FUNC) {
                        return throw_error("onCall hook expects a function");
                    }
                    elem->Hooks.onCall.push_back(right);
                } else {
                    return throw_error("Unknown hook '" + prop->_Node.ID().value + "'");
                }
            }

            return new_node(NodeType::NONE);
        }

        if (target->_Node.Op().value == ".") {
            target = eval_dot(target);
            if (prop->_Node.ID().value == "onChange") {
                if (right->type != NodeType::FUNC) {
                    return throw_error("onChange hook expects a function");
                }
                target->Hooks.onChange.push_back(right);
            } else if (prop->_Node.ID().value == "onCall") {
                if (right->type != NodeType::FUNC && target->type != NodeType::FUNC) {
                    return throw_error("onCall hook expects a function");
                }
                target->Hooks.onCall.push_back(right);
            } else {
                return throw_error("Unknown hook '" + prop->_Node.ID().value + "'");
            }

            return new_node(NodeType::NONE);
        }
    }

    return new_node(NodeType::NONE);
}

// Builtin functions

std::string Interpreter::printable(node_ptr& node) {
    switch (node->type) {
        case NodeType::NUMBER: {
            std::string num_str = std::to_string(node->_Node.Number().value);
            num_str.erase(num_str.find_last_not_of('0') + 1, std::string::npos);
            num_str.erase(num_str.find_last_not_of('.') + 1, std::string::npos);
            return num_str;
        }
        case NodeType::BOOLEAN: {
            return node->_Node.Boolean().value ? "true" : "false";
        }
        case NodeType::STRING: {
            return node->_Node.String().value;
        }
        case NodeType::FUNC: {
            std::string res = "(";
            for (int i = 0; i < node->_Node.Function().params.size(); i++) {
                node_ptr& param = node->_Node.Function().params[i];
                node_ptr& type = node->_Node.Function().param_types[param->_Node.ID().value];
                if (type) {
                    res += param->_Node.ID().value + ": " + node_repr(type);
                } else {
                    res += param->_Node.ID().value;
                }
                if (i < node->_Node.Function().params.size()-1) {
                    res += ", ";
                }
            }
            if (node->_Node.Function().return_type) {
                res += ") => " + node_repr(node->_Node.Function().return_type);
            } else {
                res += ") => ...";
            }
            if (node->_Node.Function().dispatch_functions.size() > 0) {
                res += "\n";
                for (auto& func : node->_Node.Function().dispatch_functions) {
                    res += printable(func) += "\n";
                }
            }
            return res;
        }
        case NodeType::LIST: {
            std::string res = "[";
            for (int i = 0; i < node->_Node.List().elements.size(); i++) {
                res += printable(node->_Node.List().elements[i]);
                if (i < node->_Node.List().elements.size()-1) {
                    res += ", ";
                }
            }
            res += "]";
            return res;
        }
        case NodeType::OBJECT: {
            std::string res = "{ ";
            for (auto const& elem : node->_Node.Object().properties) {
                node_ptr value = elem.second;
                res += elem.first + ": " + printable(value) + ' ';
            }
            res += "}";
            return res;
        }
        case NodeType::POINTER: {
            std::stringstream buffer;
            buffer << node->_Node.Pointer().value;
            return buffer.str();
        }
        case NodeType::LIB: {
            std::stringstream buffer;
            buffer << node->_Node.Lib().handle;
            return buffer.str();
        }
        case NodeType::NONE: {
            return "None";
        }
        case NodeType::ID: {
            return node->_Node.ID().value;
        }
        case NodeType::OP: {
            if (node->_Node.Op().value == ".") {
                return printable(node->_Node.Op().left) + "." + printable(node->_Node.Op().right);
            }
            return node->_Node.Op().value;
        }
        case NodeType::ACCESSOR: {
            return printable(node->_Node.Accessor().container) + printable(node->_Node.Accessor().accessor);
        }
        default: {
            return "<not implemented>";
        }
    }
}

void Interpreter::builtin_print(node_ptr& node) {
    std::cout << printable(node) << std::flush;
}

node_ptr Interpreter::eval_node(node_ptr& node) {

    if (node == nullptr) {
        return nullptr;
    }

    line = node->line;
    column = node->column;

    switch (node->type) {
        case NodeType::NUMBER: {
            //node->TypeInfo.type = new_node(NodeType::NUMBER);
            //node->TypeInfo.type->TypeInfo.is_type = true;
            return node;
        }
        case NodeType::STRING: {
            //node->TypeInfo.type = new_node(NodeType::STRING);
            //node->TypeInfo.type->TypeInfo.is_type = true;
            return node;
        }
        case NodeType::BOOLEAN: {
            //node->TypeInfo.type = new_node(NodeType::BOOLEAN);
            //node->TypeInfo.type->TypeInfo.is_type = true;
            return node;
        }
        case NodeType::LIST: {
            return eval_list(node);
        }
        case NodeType::OBJECT: {
            if (node->_Node.Object().elements.size() == 1 && (node->_Node.Object().elements[0]->type == NodeType::COMMA_LIST || node->_Node.Object().elements[0]->_Node.Op().value == ":")) {
                return eval_object(node);
            }
            return node;
        }
        case NodeType::ID: {
            node_ptr value = get_symbol(node->_Node.ID().value, current_symbol_table).value;
            if (value == nullptr) {
                return throw_error("Variable '" + node->_Node.ID().value + "' is undefined");
            }
            return value;
        }
        case NodeType::PAREN: {
            if (node->_Node.Paren().elements.size() != 1) {
                return throw_error("Empty parentheses");
            }
            return eval_node(node->_Node.Paren().elements[0]);
        }
        case NodeType::FUNC: {
            return eval_function(node);
        }
        case NodeType::CONSTANT_DECLARATION: {
            return eval_const_decl(node);
        }
        case NodeType::VARIABLE_DECLARATION: {
            return eval_var_decl(node);
        }
        case NodeType::FUNC_CALL: {
            return eval_func_call(node);
        }
        case NodeType::IF_STATEMENT: {
            return eval_if_statement(node);
        }
        case NodeType::IF_BLOCK: {
            return eval_if_block(node);
        }
        case NodeType::WHILE_LOOP: {
            return eval_while_loop(node);
        }
        case NodeType::FOR_LOOP: {
            return eval_for_loop(node);
        }
        case NodeType::ACCESSOR: {
            return eval_accessor(node);
        }
        case NodeType::IMPORT: {
            return eval_import(node);
        }
        case NodeType::TYPE: {
            return eval_type(node);
        }
        case NodeType::TYPE_EXT: {
            return eval_type_ext(node);
        }
        case NodeType::ENUM: {
            return eval_enum(node);
        }
        case NodeType::UNION: {
            return eval_union(node);
        }
        case NodeType::OBJECT_DECONSTRUCT: {
            return eval_object_init(node);
        }
        case NodeType::TRY_CATCH: {
            return eval_try_catch(node);
        }
        case NodeType::OP: {
            if ((node->_Node.Op().value == "+" || node->_Node.Op().value == "-") 
                && node->_Node.Op().left == nullptr) {
                return eval_pos_neg(node);
            }
            if (node->_Node.Op().value == "=") {
                return eval_eq(node);
            }
            if (node->_Node.Op().value == "+") {
                return eval_add(node);
            }
            if (node->_Node.Op().value == "-") {
                return eval_sub(node);
            }
            if (node->_Node.Op().value == "*") {
                return eval_mul(node);
            }
            if (node->_Node.Op().value == "/") {
                return eval_div(node);
            }
            if (node->_Node.Op().value == "^") {
                return eval_pow(node);
            }
            if (node->_Node.Op().value == ".") {
                return eval_dot(node);
            }
            if (node->_Node.Op().value == "%") {
                return eval_mod(node);
            }
            if (node->_Node.Op().value == "!") {
                return eval_not(node);
            }
            if (node->_Node.Op().value == "==") {
                return eval_eq_eq(node);
            }
            if (node->_Node.Op().value == "!=") {
                return eval_not_eq(node);
            }
            if (node->_Node.Op().value == "<=") {
                return eval_lt_eq(node);
            }
            if (node->_Node.Op().value == ">=") {
                return eval_gt_eq(node);
            }
            if (node->_Node.Op().value == "<") {
                return eval_lt(node);
            }
            if (node->_Node.Op().value == ">") {
                return eval_gt(node);
            }
            if (node->_Node.Op().value == "&&") {
                return eval_and(node);
            }
            if (node->_Node.Op().value == "||") {
                return eval_or(node);
            }
            if (node->_Node.Op().value == "??") {
                return eval_null_op(node);
            }
            if (node->_Node.Op().value == "&") {
                return eval_bit_and(node);
            }
            if (node->_Node.Op().value == "|") {
                return eval_bit_or(node);
            }
            if (node->_Node.Op().value == "+=") {
                return eval_plus_eq(node);
            }
            if (node->_Node.Op().value == "-=") {
                return eval_minus_eq(node);
            }
            if (node->_Node.Op().value == ";") {
                return node;
            }
            
            return throw_error("No such operator '" + node->_Node.Op().value + "'");
        }
        default: {
            return node;
        }
    }

    return node;
}

void Interpreter::evaluate() {
    while (current_node->type != NodeType::END_OF_FILE) {
        eval_node(current_node);
        advance();
    }
}

Symbol Interpreter::new_symbol(std::string name, node_ptr& value, node_ptr type) {
    Symbol symbol;
    symbol.name = name;
    symbol.value = value;
    symbol.type = type;
    return symbol;
}

Symbol Interpreter::get_symbol(std::string name, std::shared_ptr<SymbolTable>& symbol_table) {
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

    return new_symbol("_undefined_", null_value);
}

Symbol Interpreter::get_symbol_local(std::string& name, std::shared_ptr<SymbolTable>& symbol_table) {
    if (symbol_table->symbols.contains(name)) {
        return symbol_table->symbols[name];
    }

    return new_symbol("_undefined_", null_value);
}

void Interpreter::add_symbol(Symbol symbol, std::shared_ptr<SymbolTable>& symbol_table) {
    symbol_table->symbols[symbol.name] = symbol;
}

void Interpreter::delete_symbol(std::string name, std::shared_ptr<SymbolTable>& symbol_table) {
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
    node->_Node.Number().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_string_node(std::string value) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->Meta.is_const = false;
    node->_Node.String().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Interpreter::new_boolean_node(bool value) {
    auto node = std::make_shared<Node>(NodeType::BOOLEAN);
    node->Meta.is_const = false;
    node->_Node.Boolean().value = value;
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

void Interpreter::replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

void Interpreter::error_and_exit(std::string message)
{
    std::string error_message = "\nError in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message << "\n";
    while (current_symbol_table->parent != nullptr) {
        std::cout << "In '" + current_symbol_table->filename + "'" << "\n";
        current_symbol_table = current_symbol_table->parent;
    }
    exit(1);
}

node_ptr Interpreter::throw_error(std::string message)
{
    if (global_interpreter->try_catch) {
        node_ptr error = new_node(NodeType::ERROR);
        error->_Node.Error().message = message;
        return error;
    } else {
        error_and_exit(message);
        return new_node(NodeType::ERROR);
    }
}