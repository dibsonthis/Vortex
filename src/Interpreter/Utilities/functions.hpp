#pragma once
#include "../Interpreter.hpp"
#include "../../Lexer/Lexer.hpp"
#include "../../Parser/Parser.hpp"
#include "../../utils/utils.hpp"

node_ptr Interpreter::eval_function(node_ptr& node) {
    if (node->TypeInfo.is_general_type) {
        return node;
    }

    // Make a copy of function

    node_ptr func = copy_function(node);

    // Inject current scope into closure

    for (auto& symbol : current_scope->symbols) {
        func->_Node.Function().closure[symbol.first] = symbol.second;
    }

    if (func->_Node.Function().name != "") {
        if (func->_Node.Function().closure.count(func->_Node.Function().name) && func->_Node.Function().closure[func->_Node.Function().name]) {
            func->_Node.Function().closure[func->_Node.Function().name]->_Node.Function().dispatch_functions.push_back(func);
        } else {
            func->_Node.Function().closure[func->_Node.Function().name] = func;
        }
    }

    // Go through param types and evaluate them

    for (auto& param_type : func->_Node.Function().param_types) {
        param_type.second = eval_node(param_type.second);
    }

    // Go through defaults and evaluate them

    for (auto& def : func->_Node.Function().default_values) {
        def.second = eval_node(def.second);
    }

    for (auto& def : func->_Node.Function().default_values_ordered) {
        def = eval_node(def);
    }

    // Check if it's a type:
    //  - if body is an ID
    //  - if body is a type

    if (func->_Node.Function().body->type == NodeType::ID) {
        node_ptr symbol = get_symbol(func->_Node.Function().body->_Node.ID().value, current_scope);
        if (symbol) {
            func->_Node.Function().return_type = symbol;
            if (symbol->type == NodeType::OBJECT) {
                func->_Node.Function().body = new_node(NodeType::OBJECT);
                func->_Node.Function().body->_Node.Object().elements.push_back(symbol);
            } else {
                func->_Node.Function().body = symbol;
            }
        }
    } else if (func->_Node.Function().body->TypeInfo.is_type) {
        func->_Node.Function().body = eval_node(func->_Node.Function().body);
    }

    // Check if default values match the type

    for (auto& def : func->_Node.Function().default_values) {
        if (func->_Node.Function().param_types.count(def.first)) {
            if (!match_types(func->_Node.Function().param_types[def.first], def.second)) {
                return throw_error("Default value must match parameter type");
            }
        }
    }

    // Evaluate return type if it exists

    if (func->_Node.Function().return_type) {
        func->_Node.Function().return_type = eval_node(func->_Node.Function().return_type);
    }

    // Pass tags to body
    node_ptr& body = func->_Node.Function().body;
    if (body && node->Meta.tags.size() > 0) {
        if (body->type == NodeType::OBJECT) {
            for (node_ptr& e: body->_Node.Object().elements) {
                for (std::string tag : node->Meta.tags) {
                    e->Meta.tags.push_back(tag);
                }
            }
        } else {
            for (std::string tag : node->Meta.tags) {
                body->Meta.tags.push_back(tag);
            }
        }
    }

    return func;
}

node_ptr Interpreter::eval_func_call(node_ptr& node, node_ptr func = nullptr) {

    node_ptr function = new_node(NodeType::FUNC);

    if (node->_Node.FunctionCall().inline_func) {
        node_ptr inline_func_call = new_node(NodeType::FUNC_CALL);
        inline_func_call->_Node.FunctionCall().args = node->_Node.FunctionCall().args;
        return eval_func_call(inline_func_call, eval_node(node->_Node.FunctionCall().inline_func));
    }

    if (node->_Node.FunctionCall().name == "typeof") {

        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        node_ptr arg = eval_node(node->_Node.FunctionCall().args[0]);
        return get_type(arg);
    }

    if (node->_Node.FunctionCall().name == "tags") {

        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        node_ptr arg = eval_node(node->_Node.FunctionCall().args[0]);
        node_ptr tags_list = new_node(NodeType::LIST);
        for (std::string tag : arg->Meta.tags) {
            tags_list->_Node.List().elements.push_back(new_string_node(tag));
        }
        return tags_list;
    }

    if (node->_Node.FunctionCall().name == "meta") {

        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        node_ptr arg = eval_node(node->_Node.FunctionCall().args[0]);

        node_ptr meta_obj = new_node(NodeType::OBJECT);
        meta_obj->_Node.Object().properties["is_type"] = new_boolean_node(arg->TypeInfo.is_type);
        meta_obj->_Node.Object().properties["is_literal"] = new_boolean_node(arg->TypeInfo.is_literal_type);
        meta_obj->_Node.Object().properties["is_general"] = new_boolean_node(arg->TypeInfo.is_general_type);
        meta_obj->_Node.Object().properties["is_tuple"] = new_boolean_node(arg->TypeInfo.is_tuple);
        meta_obj->_Node.Object().properties["is_refinement"] = new_boolean_node(arg->TypeInfo.is_refinement_type);
        meta_obj->_Node.Object().properties["type_name"] = new_string_node(arg->TypeInfo.type_name);
        meta_obj->_Node.Object().properties["type_const"] = new_boolean_node(arg->Meta.is_const);

        return meta_obj;
    }

    if (node->_Node.FunctionCall().name == "import") {

        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        node_ptr file_path = eval_node(node->_Node.FunctionCall().args[0]);

        if (file_path->type != NodeType::STRING) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects argument 'filePath' to be a String");
        }

        std::string path = file_path->_Node.String().value + ".vtx";

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

        for (auto& symbol : import_interpreter.current_scope->symbols) {
            import_obj->_Node.Object().properties[symbol.first] = symbol.second;
        }

        return import_obj;
    }

    if (node->_Node.FunctionCall().name == "print") {
        if (node->_Node.FunctionCall().args.size() == 1) {
            node_ptr evaluated_arg = eval_node(node->_Node.FunctionCall().args[0]);
            if (tc) {
                return new_node(NodeType::NONE);
            }
            std::cout << printable(evaluated_arg) << std::flush;
        } else {
            for (node_ptr arg : node->_Node.FunctionCall().args) {
                node_ptr evaluated_arg = eval_node(arg);
                if (tc) {
                    return new_node(NodeType::NONE);
                }
                std::cout << printable(evaluated_arg) << std::flush;
            }
        }
        return new_node(NodeType::NONE);
    }

    if (node->_Node.FunctionCall().name == "println") {
        if (node->_Node.FunctionCall().args.size() == 1) {
            node_ptr evaluated_arg = eval_node(node->_Node.FunctionCall().args[0]);
            if (tc) {
                return new_node(NodeType::NONE);
            }
            std::cout << printable(evaluated_arg) << std::flush;
            std::cout << "\n";
        } else {
            for (node_ptr arg : node->_Node.FunctionCall().args) {
                node_ptr evaluated_arg = eval_node(arg);
                if (tc) {
                    return new_node(NodeType::NONE);
                }
                std::cout << printable(evaluated_arg) << std::flush;
                std::cout << '\n';
            }
        }
        return new_node(NodeType::NONE);
    }

    if (node->_Node.FunctionCall().name == "del") {

        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        node_ptr& arg = node->_Node.FunctionCall().args[0];

        if (arg->type != NodeType::ID) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects argument to be an identifier");
        }

        delete_node(arg);

        return new_node(NodeType::NONE);
    }

    if (node->_Node.FunctionCall().name == "refcount") {

        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        return new_number_node(eval_node(node->_Node.FunctionCall().args[0]).use_count());
    }

    if (node->_Node.FunctionCall().name == "error") {

        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        node_ptr arg = eval_node(node->_Node.FunctionCall().args[0]);
        std::string message = printable(arg);
        line = node->line;
        column = node->column;
        return throw_error(message);
    }

    if (node->_Node.FunctionCall().name == "string") {
        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        node_ptr arg = eval_node(node->_Node.FunctionCall().args[0]);
        return new_string_node(printable(arg));
    }

    if (node->_Node.FunctionCall().name == "number") {

        int num_args = 2;

        if (node->_Node.FunctionCall().args.size() > num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects up to " + std::to_string(num_args) + " argument(s)");
        }
        
        node_ptr var = eval_node(node->_Node.FunctionCall().args[0]);

        if (node->_Node.FunctionCall().args.size() == 2) {
            if (node->_Node.FunctionCall().args[1]->type != NodeType::NUMBER) {
                return throw_error("Function " + node->_Node.FunctionCall().name + " expects second argument to be a number");
            }
        }

        switch(var->type) {
            case NodeType::NONE: new_number_node(0);
            case NodeType::NUMBER: return var;
            case NodeType::STRING: {
                std::string value = var->_Node.String().value;
                try {
                    if (node->_Node.FunctionCall().args.size() == 2) {
                        int base = node->_Node.FunctionCall().args[1]->_Node.Number().value;
                        return new_number_node(std::stol(value, nullptr, base));
                    }
                    return new_number_node(std::stod(value));
                } catch(...) {
                    return throw_error("Cannot convert \"" + value + "\" to a number");
                }
            };
            case NodeType::BOOLEAN: {
                if (var->_Node.Boolean().value) {
                    return new_number_node(1);
                }

                return new_number_node(0);
            };
            default: return new_number_node(0);
        }
    }

    if (node->_Node.FunctionCall().name == "type") {

        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
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
            case NodeType::ERROR: return new_string_node("Error");
            default: return new_string_node("None");
        }
    }

    if (node->_Node.FunctionCall().name == "evals") {
        
        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        node_ptr var = eval_node(node->_Node.FunctionCall().args[0]);

        if (var->type != NodeType::STRING) {
            return throw_error("Function " + node->_Node.FunctionCall().name + " expects 1 string argument");
        }

        Lexer eval_lexer(var->_Node.String().value, false);
        eval_lexer.file_name = file_name;
        eval_lexer.tokenize();

        Parser eval_parser(eval_lexer.nodes, eval_lexer.file_name);
        eval_parser.parse(0, "_");
        eval_parser.remove_op_node(";");

        Interpreter eval_interpreter(eval_parser.nodes, eval_parser.file_name);
        eval_interpreter.global_interpreter = global_interpreter;
        eval_interpreter.current_scope = current_scope;
        eval_interpreter.evaluate();

        return new_node(NodeType::NONE);
    }

    if (node->_Node.FunctionCall().name == "eval") {

        int num_args = 1;

        if (node->_Node.FunctionCall().args.size() != num_args) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
        }

        node_ptr var = eval_node(node->_Node.FunctionCall().args[0]);

        if (var->type != NodeType::STRING) {
            return throw_error("Function " + node->_Node.FunctionCall().name + " expects 1 string argument");
        }

        Lexer eval_lexer(var->_Node.String().value, false);
        eval_lexer.file_name = file_name;
        eval_lexer.tokenize();

        Parser eval_parser(eval_lexer.nodes, eval_lexer.file_name);
        eval_parser.parse(0, "_");
        eval_parser.remove_op_node(";");

        Interpreter eval_interpreter(eval_parser.nodes, eval_parser.file_name);
        eval_interpreter.global_interpreter = global_interpreter;
        eval_interpreter.current_scope = current_scope;
        
        if (eval_interpreter.nodes.size() != 3) {
            return throw_error("Cannot evaluate more than one expression");
        }

        return eval_interpreter.eval_node(eval_interpreter.nodes[1]);
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

    if (func != nullptr) {
        function = copy_function(func);
    } 
    else if (!node->_Node.FunctionCall().caller) {
        node_ptr function_symbol = get_symbol(node->_Node.FunctionCall().name, current_scope);
        if (!function_symbol) {
            return throw_error("Function '" + node->_Node.FunctionCall().name + "' is undefined");
        }
        if (function_symbol->type != NodeType::FUNC) {
            return throw_error("Variable '" + node->_Node.FunctionCall().name + "' is not a function");
        }

        function = copy_function(function_symbol);

    } else {
        node_ptr method = node->_Node.FunctionCall().caller->_Node.Object().properties[node->_Node.FunctionCall().name];
        if (!method) {
            return throw_error("Method '" + node->_Node.FunctionCall().name + "' does not exist");
        }
        if (method->type != NodeType::FUNC) {
            return throw_error("Variable '" + node->_Node.FunctionCall().name + "' is not a function");
        }

        function = copy_function(method);
    }

    std::vector<node_ptr> args;
    for (node_ptr& arg : node->_Node.FunctionCall().args) {
        args.push_back(eval_node(arg));
    }

    // Type functions cannot be multiple dispatch, so we just call it here

    if (function->_Node.Function().type_function) {
        // We want to inject the type we pass in as an argument
        // To the param_types

        node_ptr func_call = new_node(NodeType::FUNC_CALL);

        // Inject any defaults

        int params_size = function->_Node.Function().params.size();
        if (args.size() < function->_Node.Function().params.size()) {
            int difference = params_size - args.size();
            auto& defaults = function->_Node.Function().default_values_ordered;
            for (int i = (defaults.size() - difference); i < defaults.size(); i++) {
                args.push_back(defaults[i]);
            }
        }

        // Typecheck

        // If function param size does not match args size, skip
        if (function->_Node.Function().params.size() != args.size()) {
            return throw_error("Type function '" + function->_Node.Function().name + "' expects " + std::to_string(function->_Node.Function().params.size()) + " arguments but " + std::to_string(args.size()) + " were provided");
        }

        symt_ptr call_scope = std::make_shared<SymbolTable>();
        call_scope->parent = current_scope;
        current_scope = call_scope;

        // Inject closure here, because the function is untyped, but may reference types from outer type if exists
        for (auto& symbol : function->_Node.Function().closure) {
            current_scope->symbols[symbol.first] = symbol.second;
        }

        for (int i = 0; i < args.size(); i++) {
            node_ptr param = function->_Node.Function().params[i];
            node_ptr param_type = eval_node(function->_Node.Function().param_types[param->_Node.ID().value]);
            if (param_type) {
                if (!match_types(param_type, args[i])) {
                    if (function->_Node.Function().params.size() != args.size()) {
                        return throw_error("Type function '" + function->_Node.Function().name + "' expects argument " + std::to_string(i) + " to be of type '" + printable(param_type));
                    }
                }
            }
            func_call->_Node.FunctionCall().args.push_back(args[i]);
            add_symbol(param->_Node.ID().value, args[i], current_scope);
        }

        node_ptr result = eval_function_direct(func_call, function);
        if (result->type == NodeType::FUNC) {
            result = eval_function(result);
        }

        current_scope = current_scope->parent;

        return result;
    }

    // Check if function args match any multiple dispatch functions

    auto functions = std::vector<node_ptr>(function->_Node.Function().dispatch_functions);
    functions.insert(functions.begin(), function);

    bool func_match = false;

    for (node_ptr& fx: functions) {

        // Inject defaults if args length < params length

        int params_size = fx->_Node.Function().params.size();
        if (args.size() < fx->_Node.Function().params.size()) {
            int difference = params_size - args.size();
            auto& defaults = fx->_Node.Function().default_values_ordered;
            for (int i = (defaults.size() - difference); i < defaults.size(); i++) {
                args.push_back(defaults[i]);
            }
        }

        // If function param size does not match args size, skip
        if (fx->_Node.Function().params.size() != args.size()) {
            continue;
        }

        for (int i = 0; i < args.size(); i++) {
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
            node_ptr arg = get_type(args[i]);
            argsStr += printable(arg);
            if (i != args.size()-1) {
                argsStr += ", ";
            }
        }
        argsStr += ")";
        return throw_error("Dispatch error in function '" + node->_Node.FunctionCall().name + "' - No function found matching args: " + argsStr + "\n\nAvailable functions:\n\n" + printable(functions[0]));
    }

    auto local_scope = std::make_shared<SymbolTable>();
    auto curr_scope = std::make_shared<SymbolTable>();

    current_scope->child = local_scope;
    local_scope->parent = current_scope;

    local_scope->child = curr_scope;
    curr_scope->parent = local_scope;

    // current_scope 
    //    -> local_scope 
    //        -> curr_scope

    if (node->_Node.FunctionCall().caller != nullptr) {
        curr_scope->symbols["this"] = node->_Node.FunctionCall().caller;
    }

    current_scope = curr_scope;
    current_scope->file_name = function->_Node.Function().name + ": " + function->_Node.Function().decl_filename;
    
    // Inject closure into local scope

    for (auto& elem : function->_Node.Function().closure) {
        add_symbol(elem.first, elem.second, current_scope);
    }

    int num_empty_args = std::count(function->_Node.Function().args.begin(), function->_Node.Function().args.end(), nullptr);

    if (args.size() > num_empty_args) {
        return throw_error("Function '" + node->_Node.FunctionCall().name + "' expects " + std::to_string(num_empty_args) + " parameters but " + std::to_string(args.size()) + " were provided");
    }

    for (int i = 0; i < args.size(); i++) {
        std::string name = function->_Node.Function().params[i]->_Node.ID().value;
        node_ptr value = args[i];
        add_symbol(name, value, current_scope);
    }

    node_ptr res = function;

    if (function->_Node.Function().body->type != NodeType::OBJECT) {
        res = eval_node(function->_Node.Function().body);
    } else {
        if (function->_Node.Function().body->_Node.Object().elements.size() == 0) {
            res = new_node(NodeType::NONE);
        }
        for (int i = 0; i < function->_Node.Function().body->_Node.Object().elements.size(); i++) {
            node_ptr expr = function->_Node.Function().body->_Node.Object().elements[i];
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::ERROR) {
                return throw_error(evaluated_expr->_Node.Error().message);
            }
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

    if (tc) {
        if (res->type == NodeType::NOVALUE) {
            if (function->_Node.Function().return_type) {
                res = function->_Node.Function().return_type;
            } else {
                res = new_node(NodeType::ANY);
            }
        }
    }

    if (!function->_Node.Function().type_function) {
        // Check against return type
        if (function->_Node.Function().return_type) {
            if (!match_types(function->_Node.Function().return_type, res)) {
                node_ptr defined_ret_type = get_type(function->_Node.Function().return_type);
                node_ptr ret_type = get_type(res);
                return throw_error("Type Error in '" + function->_Node.Function().name + "': Return type '" + printable(ret_type) + "' does not match defined return type '" + printable(defined_ret_type) + "'");
            }
        } else {
            function->_Node.Function().return_type = res;
        }
    } else {
        function->_Node.Function().return_type = res;
    }

    // OnCall Hook
    if (function->Meta.onCallFunction) {

        node_ptr return_object = new_node(NodeType::OBJECT);

        return_object->_Node.Object().properties["name"] = new_string_node(function->_Node.Function().name);
        return_object->_Node.Object().properties["params"] = new_node(NodeType::LIST);
        return_object->_Node.Object().properties["args"] = new_node(NodeType::LIST);
        return_object->_Node.Object().properties["return"] = res;

        for (node_ptr& param : function->_Node.Function().params) {
            return_object->_Node.Object().properties["params"]->_Node.List().elements.push_back(new_string_node(param->_Node.ID().value));
        }

        for (node_ptr arg : args) {
            return_object->_Node.Object().properties["args"]->_Node.List().elements.push_back(arg);
        }

        node_ptr return_object_type = get_type(return_object);
        node_ptr onCallFunction = copy_function(function->Meta.onCallFunction);
        onCallFunction = eval_function(onCallFunction);

        // Typecheck first param
        std::string param_name = onCallFunction->_Node.Function().params[0]->_Node.ID().value;
        node_ptr param_type = onCallFunction->_Node.Function().param_types[param_name];

        if (!param_type) {
            onCallFunction->_Node.Function().param_types[param_name] = return_object_type;
        } else if (param_type->type == NodeType::ANY) {
            onCallFunction->_Node.Function().param_types[param_name] = return_object_type;
        } else {
            if (!match_types(return_object_type, param_type)) {
                return throw_error("OnCallFunction's first paramater is expected to be of type '" + printable(return_object_type) + "' but was typed as '" + printable(param_type) + "'");
            }
        }

        node_ptr func_call = new_node(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().args.push_back(return_object);
        eval_func_call(func_call, onCallFunction);
    }

    current_scope = current_scope->parent->parent;
    current_scope->child->child = nullptr;
    current_scope->child = nullptr;

    return res;
}

node_ptr Interpreter::copy_function(node_ptr& func) {
    node_ptr function = new_node(NodeType::FUNC);
    function->_Node.Function().name = func->_Node.Function().name;
    function->_Node.Function().args = std::vector<node_ptr>(func->_Node.Function().args);
    function->_Node.Function().params = std::vector<node_ptr>(func->_Node.Function().params);
    function->_Node.Function().default_values = func->_Node.Function().default_values;
    function->_Node.Function().default_values_ordered = func->_Node.Function().default_values_ordered;
    function->_Node.Function().body = copy_node(func->_Node.Function().body);
    function->_Node.Function().closure = func->_Node.Function().closure;
    function->_Node.Function().is_hook = func->_Node.Function().is_hook;
    function->_Node.Function().decl_filename = func->_Node.Function().decl_filename;
    function->Meta.onCallFunction = func->Meta.onCallFunction;
    function->_Node.Function().param_types = func->_Node.Function().param_types;
    function->_Node.Function().return_type = func->_Node.Function().return_type;
    function->_Node.Function().dispatch_functions = func->_Node.Function().dispatch_functions;
    function->_Node.Function().type_function = func->_Node.Function().type_function;
    function->TypeInfo = func->TypeInfo;
    function->Meta = func->Meta;
    return function;
}

node_ptr Interpreter::eval_function_direct(node_ptr& func_call, node_ptr& function) {

    std::vector<node_ptr> args;
    for (node_ptr& arg : func_call->_Node.FunctionCall().args) {
        args.push_back(eval_node(arg));
    }

    auto local_scope = std::make_shared<SymbolTable>();
    auto curr_scope = std::make_shared<SymbolTable>();

    current_scope->child = local_scope;
    local_scope->parent = current_scope;

    local_scope->child = curr_scope;
    curr_scope->parent = local_scope;

    // current_scope 
    //    -> local_scope 
    //        -> curr_scope

    current_scope = curr_scope;
    current_scope->file_name = function->_Node.Function().name + ": " + function->_Node.Function().decl_filename;
    
    // Inject closure into local scope

    for (auto& elem : function->_Node.Function().closure) {
        add_symbol(elem.first, elem.second, curr_scope);
    }

    int num_empty_args = std::count(function->_Node.Function().args.begin(), function->_Node.Function().args.end(), nullptr);

    if (args.size() > num_empty_args) {
        return throw_error("Function '" + func_call->_Node.FunctionCall().name + "' expects " + std::to_string(num_empty_args) + " parameters but " + std::to_string(args.size()) + " were provided");
    }

    for (int i = 0; i < args.size(); i++) {
        std::string name = function->_Node.Function().params[i]->_Node.ID().value;
        node_ptr value = args[i];
        add_symbol(name, value, curr_scope);
    }

    node_ptr res = function;

    if (function->_Node.Function().body->type != NodeType::OBJECT) {
        res = eval_node(function->_Node.Function().body);
    } else {
        if (function->_Node.Function().body->_Node.Object().elements.size() == 0) {
            res = new_node(NodeType::NONE);
        }
        for (int i = 0; i < function->_Node.Function().body->_Node.Object().elements.size(); i++) {
            node_ptr expr = function->_Node.Function().body->_Node.Object().elements[i];
            node_ptr evaluated_expr = eval_node(expr);
            if (evaluated_expr->type == NodeType::ERROR) {
                return throw_error(evaluated_expr->_Node.Error().message);
            }
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

    current_scope = current_scope->parent->parent;
    current_scope->child->child = nullptr;
    current_scope->child = nullptr;

    return res;
}