#pragma once
#include "../Interpreter.hpp"

node_ptr Interpreter::eval_dot(node_ptr& node) {

    if (!node->_Node.Op().left || !node->_Node.Op().right) {
        return throw_error("Malformed '.' operation");
    }
    
    node_ptr left = eval_node(node->_Node.Op().left);
    node_ptr right = node->_Node.Op().right;

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type == NodeType::STRING) {
        return eval_dot_string(left, right);
    }

    if (left->type == NodeType::LIST) {
        return eval_dot_list(left, right);
    }

    if (left->type == NodeType::OBJECT) {
        return eval_dot_object(left, right);
    }

    if (left->type == NodeType::FUNC) {
        return eval_dot_function(left, right);
    }

    if (left->type == NodeType::LIB) {
        if (right->type == NodeType::FUNC_CALL) {
            if (right->_Node.FunctionCall().name == "call") {
                return eval_call_lib_function(left, right);
            }
        }

        return throw_error("Cannot perform '.' on types: " + node_repr(left) + ", " + node_repr(right));
    }

    if (right->type == NodeType::FUNC_CALL) {
        std::string func_name = right->_Node.FunctionCall().name;

        node_ptr func_call = std::make_shared<Node>(*right);
        func_call->_Node.FunctionCall().args.insert(func_call->_Node.FunctionCall().args.begin(), left);
        return eval_func_call(func_call);
    }

    return throw_error("Cannot perform '.' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_dot_string(node_ptr& left, node_ptr& right) {

    if (right->type == NodeType::ID) {

        std::string prop = right->_Node.ID().value;

        if (prop == "empty") {
            return new_boolean_node(left->_Node.String().value.empty());
        }

        return throw_error("String does not have property '" + prop + "'");
    }

    if (right->type == NodeType::FUNC_CALL) {

        std::string func_name = right->_Node.FunctionCall().name;

        if (right->_Node.FunctionCall().name == "length") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("String function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            return new_number_node(left->_Node.String().value.length());
        }

        if (right->_Node.FunctionCall().name == "chars") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("String function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr list = new_node(NodeType::LIST);

            for (char c : left->_Node.String().value) {
                list->_Node.List().elements.push_back(new_string_node(std::string(1, c)));
            }

            return list;
        }

        if (right->_Node.FunctionCall().name == "replace") {

            int num_args = 2;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("String function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr from_node = eval_node(right->_Node.FunctionCall().args[0]);
            node_ptr to_node = eval_node(right->_Node.FunctionCall().args[1]);

            if (from_node->type != NodeType::STRING) {
                return throw_error("String function '" + right->_Node.FunctionCall().name + "' expects argument 'from' to be a string");
            }

             if (from_node->type != NodeType::STRING) {
                return throw_error("String function '" + right->_Node.FunctionCall().name + "' expects argument 'to' to be a string");
            }

            node_ptr copy = std::make_shared<Node>(*left);

            replaceAll(copy->_Node.String().value, from_node->_Node.String().value, to_node->_Node.String().value);

            return copy;
        }
        
        node_ptr func_call = std::make_shared<Node>(*right);
        func_call->_Node.FunctionCall().args.insert(func_call->_Node.FunctionCall().args.begin(), left);
        return eval_func_call(func_call);
    }

    return throw_error("Cannot perform '.' on types: " + node_repr(left) + ", " + node_repr(right));
}

node_ptr Interpreter::eval_dot_object(node_ptr& left, node_ptr& right) {

    if (right->type == NodeType::ID) {

        if (left->_Node.Object().properties.contains(right->_Node.ID().value)) {
            return left->_Node.Object().properties[right->_Node.ID().value];
        }

        return new_node(NodeType::NONE);
    }

    if (right->type == NodeType::FUNC_CALL) {

        std::string func_name = right->_Node.FunctionCall().name;

        if (left->_Node.Object().properties.count(func_name)) {
            right->_Node.FunctionCall().caller = left;
            return eval_func_call(right);
        }

        if (func_name == "name") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("Object function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            return new_string_node(left->TypeInfo.type_name);
        }

        if (func_name == "keys") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("Object function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr keys = new_node(NodeType::LIST);

            for (auto& elem : left->_Node.Object().properties) {
                keys->_Node.List().elements.push_back(new_string_node(elem.first));
            }

            keys->TypeInfo.type = new_node(NodeType::LIST);
            keys->TypeInfo.type->_Node.List().elements.push_back(new_string_node(""));

            return keys;
        }

        if (func_name == "values") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("Object function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr values = new_node(NodeType::LIST);

            for (auto& elem : left->_Node.Object().properties) {
                values->_Node.List().elements.push_back(elem.second);
            }

            return values;
        }

        if (func_name == "items") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("Object function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr items = new_node(NodeType::LIST);

            for (auto& elem : left->_Node.Object().properties) {
                node_ptr prop = new_node(NodeType::OBJECT);
                prop->_Node.Object().properties[elem.first] = elem.second;
                items->_Node.List().elements.push_back(prop);
            }

            return items;
        }

        if (func_name == "length") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("Object function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            return new_number_node(left->_Node.Object().properties.size());
        }

        if (func_name == "contains") {

            int num_args = 1;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("Object function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr arg = eval_node(right->_Node.FunctionCall().args[0]);

            if (arg->type != NodeType::STRING) {
                return throw_error("Object function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " 'key' to be a string");
            }

            return new_boolean_node(left->_Node.Object().properties.count(arg->_Node.String().value));
        }

        // We inject object as the first arg in the function call
        // And try to run it

        node_ptr func_call = std::make_shared<Node>(*right);
        func_call->_Node.FunctionCall().args.insert(func_call->_Node.FunctionCall().args.begin(), left);
        return eval_func_call(func_call);

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

node_ptr Interpreter::eval_dot_function(node_ptr& left, node_ptr& right) {

    if (right->type == NodeType::ID) {
        std::string value = right->_Node.ID().value;
        if (value == "name") {
            return new_string_node(left->_Node.Function().name);
        }
        if (value == "params") {
            node_ptr obj = new_node(NodeType::OBJECT);
            for (node_ptr& param : left->_Node.Function().params) {
                std::string name = param->_Node.ID().value;
                obj->_Node.Object().properties[name] = left->_Node.Function().param_types.count(name) && left->_Node.Function().param_types[name] ? left->_Node.Function().param_types[name] : new_node(NodeType::ANY);
            }

            return obj;
        }
        if (value == "return") {
            if (left->_Node.Function().return_type) {
                return left->_Node.Function().return_type;
            }

            return new_node(NodeType::ANY);
        }

        return throw_error("Function does not have property '" + value + "'");
    }

    if (right->type == NodeType::FUNC_CALL) {

        node_ptr func_call = std::make_shared<Node>(*right);
        func_call->_Node.FunctionCall().args.insert(func_call->_Node.FunctionCall().args.begin(), left);
        return eval_func_call(func_call);
    }

    return throw_error("Right hand side of '.' must be an identifier, function call or accessor");
}

node_ptr Interpreter::eval_dot_list(node_ptr& left, node_ptr& right) {

    node_ptr list_type = get_type(left);

    if (right->type == NodeType::ID) {
        std::string prop = right->_Node.ID().value;
        return throw_error("List does not have property '" + prop + "'");
    }

    if (right->type == NodeType::FUNC_CALL) {
        std::string prop = right->_Node.FunctionCall().name;

        if (prop == "length") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            return new_number_node(left->_Node.List().elements.size());
        }
        if (prop == "empty") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            return new_boolean_node(left->_Node.List().elements.empty());
        }
        if (prop == "clear") {

            int num_args = 0;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            if (left->Meta.is_const) {
                return throw_error("Cannot modify constant list");
            }

            left->_Node.List().elements.clear();
            return left;
        }
        if (prop == "append") {

            int num_args = 1;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr value = eval_node(right->_Node.FunctionCall().args[0]);

            return list_method_append(left, list_type, value);
        }
        if (prop == "prepend") {

            int num_args = 1;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr value = eval_node(right->_Node.FunctionCall().args[0]);

            return list_method_prepend(left, list_type, value);
        }
        if (prop == "insert") {

            int num_args = 2;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr value = eval_node(right->_Node.FunctionCall().args[0]);
            node_ptr index_node = eval_node(right->_Node.FunctionCall().args[1]);

            if (index_node->type != NodeType::NUMBER) {
                return throw_error("List function '" + prop + "' expects 'index' to be a number");
            }

            return list_method_insert(left, list_type, value, index_node->_Node.Number().value);
        }
        if (prop == "remove_at") {

            int num_args = 1;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr index_node = eval_node(right->_Node.FunctionCall().args[0]);

            if (index_node->type != NodeType::NUMBER) {
                return throw_error("List function '" + prop + "' expects 'index' to be a number");
            }

            return list_method_remove_at(left, index_node->_Node.Number().value);
        }
        if (prop == "remove") {

            int num_args = 1;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr value = eval_node(right->_Node.FunctionCall().args[0]);

            return list_method_remove(left, value);
        }

        // Functional Operations

        if (prop == "forEach") {

            int num_args = 1;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr function = eval_node(right->_Node.FunctionCall().args[0]);

            if (function->type != NodeType::FUNC) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects a function argument");
            }

            return list_method_foreach(left, function);
        }

        if (prop == "map") {

            int num_args = 1;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr function = eval_node(right->_Node.FunctionCall().args[0]);

            if (function->type != NodeType::FUNC) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects a function argument");
            }

            return list_method_map(left, function);
        }

        if (prop == "filter") {

            int num_args = 1;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr function = eval_node(right->_Node.FunctionCall().args[0]);

            if (function->type != NodeType::FUNC) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects a function argument");
            }

            return list_method_filter(left, function);
        }

        if (prop == "reduce") {

            int num_args = 1;

            if (right->_Node.FunctionCall().args.size() != num_args) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects " + std::to_string(num_args) + " argument(s)");
            }

            node_ptr function = eval_node(right->_Node.FunctionCall().args[0]);

            if (function->type != NodeType::FUNC) {
                return throw_error("List function '" + right->_Node.FunctionCall().name + "' expects a function argument");
            }

            return list_method_reduce(left, function);
        }

        node_ptr func_call = std::make_shared<Node>(*right);
        func_call->_Node.FunctionCall().args.insert(func_call->_Node.FunctionCall().args.begin(), left);
        return eval_func_call(func_call);           
    }

    return throw_error("Right hand side of '.' must be an identifier, function call or accessor");
}

node_ptr Interpreter::list_method_append(node_ptr& list, node_ptr& list_type, node_ptr& value) {

    if (list->Meta.is_const) {
        return throw_error("Cannot modify constant list");
    }

    node_ptr type = list_type->_Node.List().elements[0];

    // Type check
    if (!match_types(type, value)) {
        node_ptr _type = get_type(value);
        return throw_error("Cannot insert value of type '" + printable(_type) + "' into container of type '" + printable(list_type) + "'");
    }

    list->_Node.List().elements.push_back(std::make_shared<Node>(*value));
    return list;
}

node_ptr Interpreter::list_method_prepend(node_ptr& list, node_ptr& list_type, node_ptr& value) {

    if (list->Meta.is_const) {
        return throw_error("Cannot modify constant list");
    }

    node_ptr type = list_type->_Node.List().elements[0];

    // Type check
    if (!match_types(type, value)) {
        node_ptr _type = get_type(value);
        return throw_error("Cannot insert value of type '" + printable(_type) + "' into container of type '" + printable(list_type) + "'");
    }

    list->_Node.List().elements.insert(list->_Node.List().elements.begin(), std::make_shared<Node>(*value));
    return list;
}

node_ptr Interpreter::list_method_insert(node_ptr& list, node_ptr& list_type, node_ptr& value, int index) {

    if (list->Meta.is_const) {
        return throw_error("Cannot modify constant list");
    }

    node_ptr type = list_type->_Node.List().elements[0];

    // Type check
    if (!match_types(type, value)) {
        node_ptr _type = get_type(value);
        return throw_error("Cannot insert value of type '" + printable(_type) + "' into container of type '" + printable(list_type) + "'");
    }

    if (index < 0) {
        index = 0;
    } else if (index > list->_Node.List().elements.size()) {
        index = list->_Node.List().elements.size();
    }

    list->_Node.List().elements.insert(list->_Node.List().elements.begin() + index, std::make_shared<Node>(*value));
    return list;
}

node_ptr Interpreter::list_method_update(node_ptr& list, node_ptr& list_type, node_ptr& value, int index) {

    if (list->Meta.is_const) {
        return throw_error("Cannot modify constant list");
    }

    node_ptr type = list_type->_Node.List().elements[0];

    // Type check
    if (!match_types(type, value)) {
        node_ptr _type = get_type(value);
        return throw_error("Cannot insert value of type '" + printable(_type) + "' into container of type '" + printable(list_type) + "'");
    }

    list->_Node.List().elements[index] = value;
    return list;
}

node_ptr Interpreter::list_method_remove_at(node_ptr& list, int index) {

    if (list->Meta.is_const) {
        return throw_error("Cannot modify constant list");
    }

    if (list->_Node.List().elements.size() == 0) {
        return list;
    }

    if (index < 0) {
        index = 0;
    } else if (index > list->_Node.List().elements.size()-1) {
        index = list->_Node.List().elements.size()-1;
    }

    list->_Node.List().elements.erase(list->_Node.List().elements.begin() + index);
    return list;
}

node_ptr Interpreter::list_method_remove(node_ptr& list, node_ptr& value) {

    if (list->Meta.is_const) {
        return throw_error("Cannot modify constant list");
    }

    for (int _index = 0; _index < list->_Node.List().elements.size(); _index++) {
        if (match_values(list->_Node.List().elements[_index], value)) {
            list->_Node.List().elements.erase(list->_Node.List().elements.begin() + _index);
            _index--;
        }
    }

    return list;
}

node_ptr Interpreter::list_method_foreach(node_ptr& list, node_ptr& function) {

    node_ptr list_type = get_type(list);
    node_ptr elem_type = list_type->_Node.List().elements[0]; // Expect Any if empty

    if (function->_Node.Function().params.size() == 0 || function->_Node.Function().params.size() > 3) {
        return throw_error("Foreach function must have between 1 to 3 parameters");
    }

    // Get first param and it's type
    std::string elem_param_name = function->_Node.Function().params[0]->_Node.ID().value;
    node_ptr elem_param_type = eval_node(function->_Node.Function().param_types[elem_param_name]);

    // If type exists, we typecheck against the inferred elem_type
    if (elem_param_type) {
        if (elem_param_type->type == NodeType::ANY) {
            function->_Node.Function().param_types[elem_param_name] = elem_type;
        } else if (!match_inferred_types(elem_param_type, elem_type)) {
            return throw_error("Foreach function expects a parameter of type '" + printable(elem_type) + "' but parameter was typed as '" + printable(elem_param_type) + "'");
        }
    } else {
        function->_Node.Function().param_types[elem_param_name] = elem_type;
    }

    // Check if second param provided, if so we should typecheck it
    if (function->_Node.Function().params.size() > 1) {
        std::string index_param_name = function->_Node.Function().params[1]->_Node.ID().value;
        node_ptr index_param_type = eval_node(function->_Node.Function().param_types[index_param_name]);

        // If type exists, we typecheck against the inferred elem_type
        if (index_param_type) {
            if (index_param_type->type == NodeType::ANY) {
                function->_Node.Function().param_types[index_param_name] = new_node(NodeType::NUMBER);
            } else if (index_param_type->type != NodeType::NUMBER) {
                return throw_error("Foreach function expects a parameter of type 'Number' but parameter was typed as '" + printable(index_param_type) + "'");
            }
        } else {
            function->_Node.Function().param_types[index_param_name] = new_node(NodeType::NUMBER);
        }
    }

    // Check if third param provided, if so we should typecheck it
    if (function->_Node.Function().params.size() > 2) {
        std::string list_param_name = function->_Node.Function().params[2]->_Node.ID().value;
        node_ptr list_param_type = eval_node(function->_Node.Function().param_types[list_param_name]);

        // If type exists, we typecheck against the inferred elem_type
        if (list_param_type) {
            if (list_param_type->type == NodeType::ANY) {
                function->_Node.Function().param_types[list_param_name] = list_type;
            } else if (!match_types(list_type, list_param_type)) {
                return throw_error("Foreach function expects a parameter of type '" + printable(list_type) + "' but parameter was typed as '" + printable(list_param_type) + "'");
            }
        } else {
            function->_Node.Function().param_types[list_param_name] = list_type;
        }
    }

    for (int _index = 0; _index < list->_Node.List().elements.size(); _index++) {
        node_ptr value = list->_Node.List().elements[_index];
        node_ptr func_call = new_node(NodeType::FUNC_CALL);

        func_call->_Node.FunctionCall().args.push_back(value);
        if (function->_Node.Function().params.size() > 1) {
            func_call->_Node.FunctionCall().args.push_back(new_number_node(_index));
        }
        if (function->_Node.Function().params.size() > 2) {
            func_call->_Node.FunctionCall().args.push_back(list);
        }

        node_ptr result = eval_func_call(func_call, function);
    }

    return new_node(NodeType::NONE);
}

node_ptr Interpreter::list_method_map(node_ptr& list, node_ptr& function) {

    node_ptr list_type = get_type(list);
    node_ptr elem_type = list_type->_Node.List().elements[0]; // Expect Any if empty

    if (function->_Node.Function().params.size() == 0 || function->_Node.Function().params.size() > 3) {
        return throw_error("Map function must have between 1 to 3 parameters");
    }

    // Get first param and it's type
    std::string elem_param_name = function->_Node.Function().params[0]->_Node.ID().value;
    node_ptr elem_param_type = eval_node(function->_Node.Function().param_types[elem_param_name]);

    // If type exists, we typecheck against the inferred elem_type
    if (elem_param_type) {
        if (elem_param_type->type == NodeType::ANY) {
            function->_Node.Function().param_types[elem_param_name] = elem_type;
        } else if (!match_inferred_types(elem_param_type, elem_type)) {
            return throw_error("Map function expects a parameter of type '" + printable(elem_type) + "' but parameter was typed as '" + printable(elem_param_type) + "'");
        }
    } else {
        function->_Node.Function().param_types[elem_param_name] = elem_type;
    }

    // Check if second param provided, if so we should typecheck it
    if (function->_Node.Function().params.size() > 1) {
        std::string index_param_name = function->_Node.Function().params[1]->_Node.ID().value;
        node_ptr index_param_type = eval_node(function->_Node.Function().param_types[index_param_name]);

        // If type exists, we typecheck against the inferred elem_type
        if (index_param_type) {
            if (index_param_type->type == NodeType::ANY) {
                function->_Node.Function().param_types[index_param_name] = new_node(NodeType::NUMBER);
            } else if (index_param_type->type != NodeType::NUMBER) {
                return throw_error("Map function expects a parameter of type 'Number' but parameter was typed as '" + printable(index_param_type) + "'");
            }
        } else {
            function->_Node.Function().param_types[index_param_name] = new_node(NodeType::NUMBER);
        }
    }

    // Check if third param provided, if so we should typecheck it
    if (function->_Node.Function().params.size() > 2) {
        std::string list_param_name = function->_Node.Function().params[2]->_Node.ID().value;
        node_ptr list_param_type = eval_node(function->_Node.Function().param_types[list_param_name]);

        // If type exists, we typecheck against the inferred elem_type
        if (list_param_type) {
            if (list_param_type->type == NodeType::ANY) {
                function->_Node.Function().param_types[list_param_name] = list_type;
            } else if (!match_types(list_type, list_param_type)) {
                return throw_error("Map function expects a parameter of type '" + printable(list_type) + "' but parameter was typed as '" + printable(list_param_type) + "'");
            }
        } else {
            function->_Node.Function().param_types[list_param_name] = list_type;
        }
    }

    node_ptr new_list = new_node(NodeType::LIST);

    for (int _index = 0; _index < list->_Node.List().elements.size(); _index++) {
        node_ptr value = list->_Node.List().elements[_index];
        node_ptr func_call = new_node(NodeType::FUNC_CALL);

        func_call->_Node.FunctionCall().args.push_back(value);
        if (function->_Node.Function().params.size() > 1) {
            func_call->_Node.FunctionCall().args.push_back(new_number_node(_index));
        }
        if (function->_Node.Function().params.size() > 2) {
            func_call->_Node.FunctionCall().args.push_back(list);
        }

        node_ptr result = eval_func_call(func_call, function);
        new_list->_Node.List().elements.push_back(result);
    }

    return new_list;
}

node_ptr Interpreter::list_method_filter(node_ptr& list, node_ptr& function) {

    node_ptr list_type = get_type(list);
    node_ptr elem_type = list_type->_Node.List().elements[0]; // Expect Any if empty

    if (function->_Node.Function().params.size() == 0 || function->_Node.Function().params.size() > 3) {
        return throw_error("Map function must have between 1 to 3 parameters");
    }

    // Get first param and it's type
    std::string elem_param_name = function->_Node.Function().params[0]->_Node.ID().value;
    node_ptr elem_param_type = eval_node(function->_Node.Function().param_types[elem_param_name]);

    // If type exists, we typecheck against the inferred elem_type
    if (elem_param_type) {
        if (elem_param_type->type == NodeType::ANY) {
            function->_Node.Function().param_types[elem_param_name] = elem_type;
        } else if (!match_inferred_types(elem_param_type, elem_type)) {
            return throw_error("Filter function expects a parameter of type '" + printable(elem_type) + "' but parameter was typed as '" + printable(elem_param_type) + "'");
        }
    } else {
        function->_Node.Function().param_types[elem_param_name] = elem_type;
    }

    // Check if second param provided, if so we should typecheck it
    if (function->_Node.Function().params.size() > 1) {
        std::string index_param_name = function->_Node.Function().params[1]->_Node.ID().value;
        node_ptr index_param_type = eval_node(function->_Node.Function().param_types[index_param_name]);

        // If type exists, we typecheck against the inferred elem_type
        if (index_param_type) {
            if (index_param_type->type == NodeType::ANY) {
                function->_Node.Function().param_types[index_param_name] = new_node(NodeType::NUMBER);
            } else if (index_param_type->type != NodeType::NUMBER) {
                return throw_error("Filter function expects a parameter of type 'Number' but parameter was typed as '" + printable(index_param_type) + "'");
            }
        } else {
            function->_Node.Function().param_types[index_param_name] = new_node(NodeType::NUMBER);
        }
    }

    // Check if third param provided, if so we should typecheck it
    if (function->_Node.Function().params.size() > 2) {
        std::string list_param_name = function->_Node.Function().params[2]->_Node.ID().value;
        node_ptr list_param_type = eval_node(function->_Node.Function().param_types[list_param_name]);

        // If type exists, we typecheck against the inferred elem_type
        if (list_param_type) {
            if (list_param_type->type == NodeType::ANY) {
                function->_Node.Function().param_types[list_param_name] = list_type;
            } else if (!match_types(list_type, list_param_type)) {
                return throw_error("Filter function expects a parameter of type '" + printable(list_type) + "' but parameter was typed as '" + printable(list_param_type) + "'");
            }
        } else {
            function->_Node.Function().param_types[list_param_name] = list_type;
        }
    }

    node_ptr new_list = new_node(NodeType::LIST);

    for (int _index = 0; _index < list->_Node.List().elements.size(); _index++) {
        node_ptr value = list->_Node.List().elements[_index];
        node_ptr func_call = new_node(NodeType::FUNC_CALL);

        func_call->_Node.FunctionCall().args.push_back(value);
        if (function->_Node.Function().params.size() > 1) {
            func_call->_Node.FunctionCall().args.push_back(new_number_node(_index));
        }
        if (function->_Node.Function().params.size() > 2) {
            func_call->_Node.FunctionCall().args.push_back(list);
        }

        node_ptr result = eval_func_call(func_call, function);

        if (result->type != NodeType::BOOLEAN) {
            return throw_error("Filter function must return a boolean value");
        }

        if (result->_Node.Boolean().value) {
            new_list->_Node.List().elements.push_back(value);
        }
    }

    return new_list;
}

node_ptr Interpreter::list_method_reduce(node_ptr& list, node_ptr& function) {

    if (list->_Node.List().elements.size() < 2) {
        return list;
    }

    node_ptr list_type = get_type(list);
    node_ptr elem_type = list_type->_Node.List().elements[0]; // Expect Any if empty

    if (function->_Node.Function().params.size() < 2 || function->_Node.Function().params.size() > 4) {
        return throw_error("Reduce function must have between 2 to 4 parameters");
    }

    // Get first param and it's type
    std::string elem_a_param_name = function->_Node.Function().params[0]->_Node.ID().value;
    node_ptr elem_a_param_type = eval_node(function->_Node.Function().param_types[elem_a_param_name]);

    // If type exists, we typecheck against the inferred elem_type
    if (elem_a_param_type) {
        if (elem_a_param_type->type == NodeType::ANY) {
            function->_Node.Function().param_types[elem_a_param_name] = elem_type;
        } else if (!match_inferred_types(elem_a_param_type, elem_type)) {
            return throw_error("Reduce function expects a parameter of type '" + printable(elem_type) + "' but parameter was typed as '" + printable(elem_a_param_type) + "'");
        }
    } else {
        function->_Node.Function().param_types[elem_a_param_name] = elem_type;
    }

    // Get second param and it's type
    std::string elem_b_param_name = function->_Node.Function().params[1]->_Node.ID().value;
    node_ptr elem_b_param_type = eval_node(function->_Node.Function().param_types[elem_b_param_name]);

    // If type exists, we typecheck against the inferred elem_type
    if (elem_b_param_type) {
        if (elem_b_param_type->type == NodeType::ANY) {
            function->_Node.Function().param_types[elem_b_param_name] = elem_type;
        } else if (!match_inferred_types(elem_b_param_type, elem_type)) {
            return throw_error("Reduce function expects a parameter of type '" + printable(elem_type) + "' but parameter was typed as '" + printable(elem_b_param_type) + "'");
        }
    } else {
        function->_Node.Function().param_types[elem_b_param_name] = elem_type;
    }

    // Check if third param provided, if so we should typecheck it
    if (function->_Node.Function().params.size() > 2) {
        std::string index_param_name = function->_Node.Function().params[2]->_Node.ID().value;
        node_ptr index_param_type = eval_node(function->_Node.Function().param_types[index_param_name]);

        // If type exists, we typecheck against the inferred elem_type
        if (index_param_type) {
            if (index_param_type->type == NodeType::ANY) {
                function->_Node.Function().param_types[index_param_name] = new_node(NodeType::NUMBER);
            } else if (index_param_type->type != NodeType::NUMBER) {
                return throw_error("Reduce function expects a parameter of type 'Number' but parameter was typed as '" + printable(index_param_type) + "'");
            }
        } else {
            function->_Node.Function().param_types[index_param_name] = new_node(NodeType::NUMBER);
        }
    }

    // Check if third param provided, if so we should typecheck it
    if (function->_Node.Function().params.size() > 3) {
        std::string list_param_name = function->_Node.Function().params[3]->_Node.ID().value;
        node_ptr list_param_type = eval_node(function->_Node.Function().param_types[list_param_name]);

        // If type exists, we typecheck against the inferred elem_type
        if (list_param_type) {
            if (list_param_type->type == NodeType::ANY) {
                function->_Node.Function().param_types[list_param_name] = list_type;
            } else if (!match_types(list_type, list_param_type)) {
                return throw_error("Reduce function expects a parameter of type '" + printable(list_type) + "' but parameter was typed as '" + printable(list_param_type) + "'");
            }
        } else {
            function->_Node.Function().param_types[list_param_name] = list_type;
        }
    }

    node_ptr new_list = new_node(NodeType::LIST);

    for (int _index = 0; _index < list->_Node.List().elements.size()-1; _index++) {
        node_ptr value = list->_Node.List().elements[_index];
        node_ptr next_value = list->_Node.List().elements[_index + 1];

        if (new_list->_Node.List().elements.size() > 0) {
            value = new_list->_Node.List().elements[0];
        }

        node_ptr func_call = new_node(NodeType::FUNC_CALL);

        func_call->_Node.FunctionCall().args.push_back(value);
        func_call->_Node.FunctionCall().args.push_back(next_value);

        if (function->_Node.Function().params.size() > 2) {
            func_call->_Node.FunctionCall().args.push_back(new_number_node(_index));
        }
        if (function->_Node.Function().params.size() > 3) {
            func_call->_Node.FunctionCall().args.push_back(list);
        }

        node_ptr result = eval_func_call(func_call, function);
        value = result;

        if (new_list->_Node.List().elements.size() == 0) {
            new_list->_Node.List().elements.push_back(result);
        } else {
            new_list->_Node.List().elements[0] = result;
        }
    }

    return new_list->_Node.List().elements[0];
}

bool Interpreter::match_inferred_types(node_ptr& nodeA, node_ptr& nodeB) {
    if (nodeA->type == NodeType::PIPE_LIST && nodeB->type == NodeType::PIPE_LIST) {
        return match_values(nodeA, nodeB);
    } else {
        return match_types(nodeA, nodeB);
    }
}