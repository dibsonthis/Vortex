#pragma once
#include "../Typechecker.hpp"

node_ptr Typechecker::tc_eq(node_ptr& node) {
    node_ptr left = node->_Node.Op().left;
    node_ptr right = tc_node(node->_Node.Op().right);

    bool is_ref = right->type == NodeType::REF;

    if (is_ref) {
        right = tc_node(right->_Node.Ref().value);
    }

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ID) {
        return tc_eq_id(left, right, is_ref);
    }

    if (left->type == NodeType::OP && left->_Node.Op().value == ".") {
        return tc_eq_dot(left, right, is_ref);
    }

    if (left->type == NodeType::ACCESSOR) {
        return tc_eq_accessor(left, right, is_ref);
    }

    return throw_error("Malformed '=' operation");
}

node_ptr Typechecker::tc_eq_id(node_ptr& id, node_ptr& value, bool is_ref) {
    std::string var_name = id->_Node.ID().value;
    node_ptr var = get_symbol(var_name, current_scope);
    if (!var) {
        return throw_error("Variable '" + var_name + "' is undefined");
    }

    // Check if const
    if (var->Meta.is_const) {
        return throw_error("Cannot modify constant '" + var_name + "'");
    }

    node_ptr type = var;

    if (var->TypeInfo.type) {
        type = var->TypeInfo.type;
    }

    if (!match_types(type, value)) {
        return throw_error("Variable '" + var_name + "' expects value of type '" + node_repr(type) + "' but received value of type '" + node_repr(value) + "'");
    }

    // Capture current state
    node_ptr old = std::make_shared<Node>(*var);

    // Evaluate Hooks

    if (var->Meta.onChangeFunction) {
        node_ptr on_change_object = new_node(NodeType::OBJECT);
        on_change_object->_Node.Object().properties["name"] = new_string_node(var_name);
        on_change_object->_Node.Object().properties["old"] = old;
        on_change_object->_Node.Object().properties["new"] = var;

        node_ptr on_change_object_type = get_type(on_change_object);
        node_ptr hook = copy_function(var->Meta.onChangeFunction);
        hook = tc_function(hook);

        // Typecheck first param
        std::string param_name = hook->_Node.Function().params[0]->_Node.ID().value;
        node_ptr param_type = hook->_Node.Function().param_types[param_name];

        if (!param_type || param_type->type == NodeType::ANY) {
            hook->_Node.Function().param_types[param_name] = on_change_object_type;
        } else {
            if (!match_types(on_change_object_type, param_type)) {
                return throw_error("Hook's first paramater is expected to be of type '" + printable(on_change_object_type) + "' but was typed as '" + printable(param_type) + "'");
            }
        }

        node_ptr func_call = new_node(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().args.push_back(on_change_object);
        tc_func_call(func_call, hook);
    }

    return var;
}

node_ptr Typechecker::tc_eq_dot(node_ptr& left, node_ptr& value, bool is_ref) {

    node_ptr object = tc_node(left->_Node.Op().left);

    if (object->type == NodeType::PIPE_LIST) {
        node_ptr list = new_node(NodeType::LIST);
        list->type = NodeType::PIPE_LIST;

        for (node_ptr& elem : object->_Node.List().elements) {
            node_ptr dot_op = new_node(NodeType::OP);
            dot_op->_Node.Op().value = ".";
            dot_op->_Node.Op().left = elem;
            dot_op->_Node.Op().right = left->_Node.Op().right;
            node_ptr res = tc_eq_dot(dot_op, value, is_ref);
            res->TypeInfo.is_type = true;
            list->_Node.List().elements.push_back(res);
        }

        list->_Node.List().elements = sort_and_unique(list->_Node.List().elements);
        if (list->_Node.List().elements.size() == 1) {
            return list->_Node.List().elements[0];
        }
        return list;
    }

    if (object->type != NodeType::OBJECT) {
        return throw_error("Left hand side of '.' must be an object");
    }

    if (object->Meta.is_const) {
        return throw_error("Cannot modify constant object");
    }

    node_ptr prop = left->_Node.Op().right;

    if (prop->type != NodeType::ID && prop->type != NodeType::ACCESSOR) {
        return throw_error("Right hand side of '.' must be an identifier");
    }

    if (prop->type == NodeType::ACCESSOR) {
        node_ptr accessed_value = tc_dot(left);

        // Type check
        if (!match_types(accessed_value, value)) {
            node_ptr _type = get_type(accessed_value);
            return throw_error("Cannot modify object property type '" + printable(_type) + "'");
        }

        if (accessed_value->type == NodeType::NONE && prop->_Node.Accessor().container->TypeInfo.type_name != "") {
            return throw_error("Property '" + prop->_Node.ID().value + "' does not exist on type '" + object->TypeInfo.type_name + "'");
        }

        // Capture current state
        node_ptr old = std::make_shared<Node>(*accessed_value);

        // Evaluate Hooks

        if (accessed_value->Meta.onChangeFunction) {
            node_ptr on_change_object = new_node(NodeType::OBJECT);
            std::string name = printable(left->_Node.Op().left) + "." + printable(prop->_Node.Accessor().accessor);
            on_change_object->_Node.Object().properties["name"] = new_string_node(name);
            on_change_object->_Node.Object().properties["old"] = old;
            on_change_object->_Node.Object().properties["new"] = accessed_value;

            node_ptr on_change_object_type = get_type(on_change_object);
            node_ptr hook = copy_function(accessed_value->Meta.onChangeFunction);
            hook = tc_function(hook);

            // Typecheck first param
            std::string param_name = hook->_Node.Function().params[0]->_Node.ID().value;
            node_ptr param_type = hook->_Node.Function().param_types[param_name];

            if (!param_type || param_type->type == NodeType::ANY) {
                hook->_Node.Function().param_types[param_name] = on_change_object_type;
            } else {
                if (!match_types(on_change_object_type, param_type)) {
                    return throw_error("Hook's first paramater is expected to be of type '" + printable(on_change_object_type) + "' but was typed as '" + printable(param_type) + "'");
                }
            }

            node_ptr func_call = new_node(NodeType::FUNC_CALL);
            func_call->_Node.FunctionCall().args.push_back(on_change_object);
            tc_func_call(func_call, hook);
        }

        return object;
    }

    node_ptr accessed_value = object->_Node.Object().properties[prop->_Node.ID().value];

    if (!accessed_value) {
        // If the object has a type and the property wasn't found, we error
        if (object->TypeInfo.type_name != "") {
            return throw_error("Property '" + prop->_Node.ID().value + "' does not exist on type '" + object->TypeInfo.type_name + "'");
        }
        accessed_value = new_node(NodeType::NONE);
    }

    // Type check
    if (!match_types(accessed_value, value)) {
        node_ptr _type = get_type(accessed_value);
        return throw_error("Type error in property '" + prop->_Node.ID().value + "' - Cannot modify object property type '" + printable(_type) + "'");
    }

    // Capture current state
    node_ptr old = std::make_shared<Node>(*accessed_value);

    // Evaluate Hooks

    if (accessed_value->Meta.onChangeFunction) {
        node_ptr on_change_object = new_node(NodeType::OBJECT);
        std::string name = printable(left->_Node.Op().left) + "." + prop->_Node.ID().value;
        on_change_object->_Node.Object().properties["name"] = new_string_node(name);
        on_change_object->_Node.Object().properties["old"] = old;
        on_change_object->_Node.Object().properties["new"] = accessed_value;

        node_ptr on_change_object_type = get_type(on_change_object);
        node_ptr hook = copy_function(accessed_value->Meta.onChangeFunction);
        hook = tc_function(hook);

        // Typecheck first param
        std::string param_name = hook->_Node.Function().params[0]->_Node.ID().value;
        node_ptr param_type = hook->_Node.Function().param_types[param_name];

        if (!param_type || param_type->type == NodeType::ANY) {
            hook->_Node.Function().param_types[param_name] = on_change_object_type;
        } else {
            if (!match_types(on_change_object_type, param_type)) {
                return throw_error("Hook's first paramater is expected to be of type '" + printable(on_change_object_type) + "' but was typed as '" + printable(param_type) + "'");
            }
        }

        node_ptr func_call = new_node(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().args.push_back(on_change_object);
        tc_func_call(func_call, hook);
    }
    
    return object;
}

node_ptr Typechecker::tc_eq_accessor(node_ptr& left, node_ptr& value, bool is_ref) {
    
    node_ptr container = tc_node(left->_Node.Accessor().container);
    node_ptr accessor = tc_node(left->_Node.Accessor().accessor);

    if (container->Meta.is_const) {
        return throw_error("Cannot modify constant");
    }

    if (container->type == NodeType::LIST) {

        accessor = tc_node(accessor->_Node.List().elements[0]);

        if (accessor->type != NodeType::NUMBER) {
            return throw_error("List accessor must be a number");
        }

        node_ptr accessed_value = tc_accessor(left);

        node_ptr _type = get_type(container);

        // Capture copy of current list
        node_ptr old_list = new_node(NodeType::LIST);
        old_list->Meta = container->Meta;
        old_list->TypeInfo = container->TypeInfo;
        for (node_ptr& elem : container->_Node.List().elements) {
            old_list->_Node.List().elements.push_back(std::make_shared<Node>(*elem));
        }

        // Evaluate List Hooks

        if (container->Meta.onChangeFunction) {
            node_ptr on_change_object = new_node(NodeType::OBJECT);
            std::string name = printable(left->_Node.Accessor().container);
            on_change_object->_Node.Object().properties["name"] = new_string_node(name);
            on_change_object->_Node.Object().properties["old"] = old_list;
            on_change_object->_Node.Object().properties["new"] = container;

            node_ptr on_change_object_type = get_type(on_change_object);
            node_ptr hook = copy_function(container->Meta.onChangeFunction);
            hook = tc_function(hook);

            // Typecheck first param
            std::string param_name = hook->_Node.Function().params[0]->_Node.ID().value;
            node_ptr param_type = hook->_Node.Function().param_types[param_name];

            if (!param_type || param_type->type == NodeType::ANY) {
                hook->_Node.Function().param_types[param_name] = on_change_object_type;
            } else {
                if (!match_types(on_change_object_type, param_type)) {
                    return throw_error("Hook's first paramater is expected to be of type '" + printable(on_change_object_type) + "' but was typed as '" + printable(param_type) + "'");
                }
            }

            node_ptr func_call = new_node(NodeType::FUNC_CALL);
            func_call->_Node.FunctionCall().args.push_back(on_change_object);
            tc_func_call(func_call, hook);
        }

        return value;
    }
    
    if (container->type == NodeType::OBJECT) {

        container = std::make_shared<Node>(*container);

        node_ptr accessed_value = tc_accessor(left);

        // If the object has a type and the property wasn't found, we error
        if (container->TypeInfo.type_name != "" && accessed_value->type == NodeType::NONE) {
            return throw_error("Property does not exist on type '" + container->TypeInfo.type_name + "'");
        }  

        if (accessed_value->type == NodeType::ANY) {
            // Check if object has a signature, and typecheck against it
            if (container->TypeInfo.type && container->TypeInfo.type->_Node.Object().elements.size() == 1) {
                node_ptr index_type = tc_node(container->TypeInfo.type->_Node.Object().elements[0]);
                if (!match_types(index_type, value)) {
                    node_ptr _type = get_type(value);
                    return throw_error("Cannot add object property of type '" + printable(_type) + "', accepted types are: " + printable(index_type), value);
                }
            }
            container->_Node.Object().properties[accessor->_Node.List().elements[0]->_Node.String().value] = value;
            return value;
        }

        if (!match_types(accessed_value, value)) {
            node_ptr _type = get_type(accessed_value);
            return throw_error("Cannot modify object property type '" + printable(_type) + "'");
        }

        // Warning: This may result in duplicates when we reach runtime

        if (accessed_value->type == NodeType::FUNC && value->type == NodeType::FUNC) {
            accessed_value->_Node.Function().dispatch_functions.push_back(value);
            return value;
        }

        // Capture current state
        node_ptr old = std::make_shared<Node>(*accessed_value);

        // ------  Evaluate Property Hooks ------ //

        if (accessed_value->Meta.onChangeFunction) {
            node_ptr on_change_object = new_node(NodeType::OBJECT);
            std::string name = printable(left);
            on_change_object->_Node.Object().properties["name"] = new_string_node(name);
            on_change_object->_Node.Object().properties["old"] = old;
            on_change_object->_Node.Object().properties["new"] = accessed_value;

            node_ptr on_change_object_type = get_type(on_change_object);
            node_ptr hook = copy_function(accessed_value->Meta.onChangeFunction);
            hook = tc_function(hook);

            // Typecheck first param
            std::string param_name = hook->_Node.Function().params[0]->_Node.ID().value;
            node_ptr param_type = hook->_Node.Function().param_types[param_name];

            if (!param_type || param_type->type == NodeType::ANY) {
                hook->_Node.Function().param_types[param_name] = on_change_object_type;
            } else {
                if (!match_types(on_change_object_type, param_type)) {
                    return throw_error("Hook's first paramater is expected to be of type '" + printable(on_change_object_type) + "' but was typed as '" + printable(param_type) + "'");
                }
            }

            node_ptr func_call = new_node(NodeType::FUNC_CALL);
            func_call->_Node.FunctionCall().args.push_back(on_change_object);
            tc_func_call(func_call, hook);
        }

        return value;
    }

    node_ptr container_type = get_type(container);

    return throw_error("Cannot index into variable of type '" + printable(container_type) + "'");
}