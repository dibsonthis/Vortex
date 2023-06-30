#pragma once
#include "../Typechecker.hpp"

node_ptr Typechecker::tc_object(node_ptr& node) {

    node_ptr object = new_node(NodeType::OBJECT);
    object->TypeInfo = node->TypeInfo;
    object->Meta = node->Meta;

    if (node->_Node.Object().elements.size() == 0) {
        return object;
    }

    node_ptr elem_list;

    if (node->_Node.Object().elements.size() == 1 && node->_Node.Object().elements[0]->type != NodeType::COMMA_LIST) {
        elem_list = new_node(NodeType::LIST);
        elem_list->type = NodeType::COMMA_LIST;
        elem_list->_Node.List().elements.push_back(node->_Node.Object().elements[0]);
    } else {
        elem_list = node->_Node.Object().elements[0];
    }

    for (node_ptr& elem : elem_list->_Node.List().elements) {
        if (elem->type != NodeType::OP || elem->_Node.Op().value != ":") {
            return throw_error("Object properties must be in the shape of 'key: value'");
        }

        // If function, add "this"
        if (elem->_Node.Op().right->type == NodeType::FUNC) {
            elem->_Node.Op().right->_Node.Function().closure["this"] = object;
        }

        node_ptr left = elem->_Node.Op().left;
        node_ptr right = elem->_Node.Op().right;
        if (elem->_Node.Op().right->type != NodeType::FUNC) {
            right = tc_node(elem->_Node.Op().right);
        }

        std::string prop_name;

        if (left->type == NodeType::LIST && left->_Node.List().elements.size() == 1) {
            node_ptr literal_name = tc_node(left->_Node.List().elements[0]);
            
            if (literal_name->type != NodeType::STRING) {
                return throw_error("Literal property name must evaluate to a string");
            }

            prop_name = literal_name->_Node.String().value;
        } 
        
        else if (left->type == NodeType::ID) {
            prop_name = left->_Node.ID().value;
        }

        object->_Node.Object().properties[prop_name] = right;

        if (object->_Node.Object().properties[prop_name]->type == NodeType::FUNC) {
            object->_Node.Object().properties[prop_name]->_Node.Function().closure["this"] = object;
            object->_Node.Object().properties[prop_name] = tc_function(object->_Node.Object().properties[prop_name]);
        }
    }

    return object;
}

node_ptr Typechecker::tc_list(node_ptr& node) {

    if (node->Meta.evaluated) {
        return node;
    }

    node_ptr list = new_node(NodeType::LIST);
    list->TypeInfo = node->TypeInfo;
    list->Meta = node->Meta;

    node_ptr elem_list;

    if (node->_Node.List().elements.size() == 0) {
        elem_list = list;
    } else if (node->_Node.List().elements.size() == 1 && node->_Node.List().elements[0]->type != NodeType::COMMA_LIST) {
        elem_list = new_node(NodeType::LIST);
        elem_list->type = NodeType::COMMA_LIST;
        elem_list->_Node.List().elements.push_back(node->_Node.List().elements[0]);
    } else if (node->_Node.List().elements.size() == 1 && node->_Node.List().elements[0]->type == NodeType::COMMA_LIST) {
        elem_list = node->_Node.List().elements[0];
    } else {
        elem_list = node;
    }

    for (node_ptr& elem : elem_list->_Node.List().elements) {
        list->_Node.List().elements.push_back(tc_node(elem));
    }

    list->TypeInfo.type = get_type(list);
    list->Meta.evaluated = true;

    return list;
}

node_ptr Typechecker::tc_pipe_list(node_ptr& node) {
    node_ptr list = new_node(NodeType::LIST);
    list->type = NodeType::PIPE_LIST;
    list->TypeInfo = node->TypeInfo;
    list->Meta = node->Meta;

    for (node_ptr& elem : node->_Node.List().elements) {
        node_ptr evaluated_elem = tc_node(elem);
        if (!evaluated_elem->TypeInfo.is_type) {
            if (evaluated_elem->type != NodeType::OBJECT && evaluated_elem->type != NodeType::LIST && evaluated_elem->type != NodeType::FUNC) {
                evaluated_elem->TypeInfo.is_literal_type = true;
            }
        }
        if (evaluated_elem->type == NodeType::PIPE_LIST) {
            for (node_ptr& elem : evaluated_elem->_Node.List().elements) {
                list->_Node.List().elements.push_back(tc_node(elem));
            }
        } else {
            list->_Node.List().elements.push_back(evaluated_elem);
        }
    }

    list->_Node.List().elements = sort_and_unique(list->_Node.List().elements);

    if (list->_Node.List().elements.size() == 1) {
        return list->_Node.List().elements[0];
    }

    return list;
}

node_ptr Typechecker::tc_accessor(node_ptr& node) {
    
    node_ptr container = tc_node(node->_Node.Accessor().container);
    node_ptr accessor = tc_node(node->_Node.Accessor().accessor);

    if (accessor->_Node.List().elements.size() != 1) {
        return throw_error("Malformed accessor");
    }

    if (container->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (container->type == NodeType::STRING) {
        node_ptr index_node = tc_node(accessor->_Node.List().elements[0]);
        if (index_node->type != NodeType::NUMBER) {
            return throw_error("String accessor expects a number");
        }
        int index = index_node->_Node.Number().value;
        return new_string_node("");
    }

    if (container->type == NodeType::LIST) {
        node_ptr index_node = tc_node(accessor->_Node.List().elements[0]);
        if (index_node->type != NodeType::NUMBER) {
            return throw_error("List accessor expects a number");
        }
        int index = index_node->_Node.Number().value;

        if (container->Meta.is_const) {

            if (index >= container->_Node.List().elements.size() || index < 0) {
                return new_node(NodeType::ANY);
            }

            return container->_Node.List().elements[index];
        }

        node_ptr list_type = get_type(container);
        return list_type->_Node.List().elements[0];
    }

    if (container->type == NodeType::OBJECT) {
        node_ptr prop_node = tc_node(accessor->_Node.List().elements[0]);

        if (prop_node->type == NodeType::ANY) {
            return new_node(NodeType::ANY);
        }

        if (prop_node->type == NodeType::PIPE_LIST) {
            for (node_ptr& elem: prop_node->_Node.List().elements)  {
                if (elem->type != NodeType::STRING) {
                    return throw_error("Object accessor expects a string");
                }
                if (container->_Node.Object().properties.contains(elem->_Node.String().value)) {
                    return container->_Node.Object().properties[elem->_Node.String().value];
                }
            }
            return new_node(NodeType::ANY);
        }

        if (prop_node->type != NodeType::STRING) {
            return throw_error("Object accessor expects a string");
        }
        std::string prop = prop_node->_Node.String().value;
        if (container->_Node.Object().properties.contains(prop)) {
            return container->_Node.Object().properties[prop];
        }
        return new_node(NodeType::ANY);
    }

    return throw_error("Value of type '" + node_repr(container) + "' is not accessable");
}

node_ptr Typechecker::tc_type(node_ptr& node) {

    if (node->_Node.Type().parametric_type) {
        node_ptr func = new_node(NodeType::FUNC);
        func->_Node.Function().name = node->_Node.Type().name;
        func->_Node.Function().params = std::vector<node_ptr>(node->_Node.Type().params);
        func->_Node.Function().param_types = node->_Node.Type().param_types;
        func->_Node.Function().body = copy_node(node->_Node.Type().body);
        for (int i = 0; i < node->_Node.Type().params.size(); i++) {
            func->_Node.Function().args.push_back(nullptr);
        }

        // Typecheck here for structural weirdnesses
        node_ptr func_copy = copy_function(func);
        store_func_type = false;
        tc_function(func_copy);
        store_func_type = true;

        func->_Node.Function().type_function = true;

        add_symbol(func->_Node.Function().name, func, current_scope);
        return func;
    }

    // We add it to the scope so that it can be self-referential

    node_ptr object = new_node(NodeType::OBJECT);
    object->TypeInfo.is_type = true;
    object->TypeInfo.type_name = node->_Node.Type().name;
    object->TypeInfo.is_decl = node->TypeInfo.is_decl;
    add_symbol(node->_Node.Type().name, object, current_scope);

    if (object->TypeInfo.is_decl) {
        return object;
    }

    node_ptr body = node->_Node.Type().body;

    // Check refinement type

    if (body->type == NodeType::FUNC) {
        body->TypeInfo.is_refinement_type = true;
        // Test
        body->_Node.Function().type_function = true;
        *object = *body;
        return object;
    }

    // If object

    if (body->type == NodeType::OBJECT) {

        // If empty type

        if (body && body->_Node.Object().elements.size() == 0) {
            return object;
        }

        node_ptr elems = body->_Node.Object().elements[0];

        if (elems->type != NodeType::COMMA_LIST) {
            node_ptr comma_list = new_node(NodeType::LIST);
            comma_list->type = NodeType::COMMA_LIST;
            comma_list->_Node.List().elements.push_back(body->_Node.Object().elements[0]);
            elems = comma_list;
        }

        for (node_ptr prop : elems->_Node.List().elements) {

            node_ptr default_val = nullptr;

            if (prop->type == NodeType::ID) {
                // This is essentually an any type
                object->_Node.Object().properties[prop->_Node.ID().value] = new_node(NodeType::ANY);
                object->_Node.Object().defaults[prop->_Node.ID().value] = new_node(NodeType::ANY);
                continue;
            }
            else if (prop->type == NodeType::OP && prop->_Node.Op().value == "=" && prop->_Node.Op().left->type == NodeType::ID) {
                // This is essentually an any type
                std::string name = prop->_Node.Op().left->_Node.ID().value;

                if (prop->_Node.Op().right->type == NodeType::FUNC) {
                    prop->_Node.Op().right->_Node.Function().closure["this"] = object;
                }

                node_ptr value = tc_node(prop->_Node.Op().right);
                object->_Node.Object().properties[name] = new_node(NodeType::ANY);

                if (value->type == NodeType::FUNC && object->_Node.Object().defaults.count(name) && object->_Node.Object().defaults[name]->type == NodeType::FUNC) {
                    // If it's a function, add it as a dispatch
                    object->_Node.Object().defaults[name]->_Node.Function().dispatch_functions.push_back(value);
                } else {
                    if (name == "_init") {
                        object->_Node.Object().properties[name] = value;
                    }
                    object->_Node.Object().defaults[name] = value;
                }
                continue;
            } else if (prop->type == NodeType::OP && prop->_Node.Op().value == "=") {
                if (prop->_Node.Op().right->type == NodeType::FUNC) {
                    prop->_Node.Op().right->_Node.Function().closure["this"] = object;
                }
                default_val = tc_node(prop->_Node.Op().right);
                prop = prop->_Node.Op().left; // prop is now :
            }

            if (prop->type == NodeType::OP && prop->_Node.Op().value != ":") {
                return throw_error("Object properties must be in the shape of 'key: value (= default)'");
            }

            if (prop->type != NodeType::OP) {
                error_and_exit("Malformed type property");
            }

            node_ptr prop_name = prop->_Node.Op().left;
            std::string name;

            if (prop_name->type == NodeType::LIST && prop_name->_Node.List().elements.size() == 1) {
                node_ptr literal_name = tc_node(prop_name->_Node.List().elements[0]);
                if (literal_name->type != NodeType::STRING) {
                    return throw_error("Literal property name must evaluate to a string");
                }

                name = literal_name->_Node.String().value;
            } else if (prop_name->type == NodeType::ID) {
                name = prop_name->_Node.ID().value;
            } else {
                return throw_error("Property names must be identifiers or literals");
            }

            node_ptr type_value = tc_node(prop->_Node.Op().right);
            type_value->TypeInfo.is_type = true;

            object->_Node.Object().properties[name] = type_value;

            if (default_val) {
                if (!type_value->TypeInfo.is_decl && !match_types(type_value, default_val)) {
                    return throw_error("Default type constructor for propery '" 
                    + name + "' does not match type: Expecting value of type '" 
                    + printable(type_value) + "' but received value of type '" 
                    + printable(default_val) + "'");
                }
                if (default_val->type == NodeType::FUNC && object->_Node.Object().defaults.count(name) && object->_Node.Object().defaults[name]->type == NodeType::FUNC) {
                    object->_Node.Object().defaults[name]->_Node.Function().dispatch_functions.push_back(default_val);
                } else {
                    object->_Node.Object().defaults[name] = default_val;
                }
            }
        }

        object->TypeInfo.type_name = node->_Node.Type().name;

        return object;
    }

    node_ptr res = tc_node(body);
    if (res->type == NodeType::OBJECT) {
        res->TypeInfo.type_name = node->_Node.Type().name;
    }
    if (res->type == NodeType::FUNC) {
        res->_Node.Function().name = node->_Node.Type().name;
        res->_Node.Function().type_function = true;
    }
    if (res->type == NodeType::PIPE_LIST) {
        res->TypeInfo.type_name = node->_Node.Type().name;
    }
    res->TypeInfo.is_type = true;
    *object = *res;
    return object;
}

node_ptr Typechecker::tc_object_init(node_ptr& node) {
    node_ptr type = get_symbol(node->_Node.ObjectDeconstruct().name, current_scope);

    if (type == nullptr) {
        return throw_error("Type '" + node->_Node.ObjectDeconstruct().name + "' is undefined");
    }

    if (!type->TypeInfo.is_type) {
        return throw_error("Variable '" + node->_Node.ObjectDeconstruct().name + "' is not a type");
    }

    node_ptr object = new_node(NodeType::OBJECT);
    object->TypeInfo.type_name = node->_Node.ObjectDeconstruct().name;
    node_ptr init_props = tc_object(node->_Node.ObjectDeconstruct().body);

    // Add defaults if they exist

    for (auto def : type->_Node.Object().defaults) {
        if (def.first == "_init") {
            continue;
        }
        object->_Node.Object().properties[def.first] = std::make_shared<Node>(*def.second);
    }

    // Add Init props

    for (auto prop : init_props->_Node.Object().properties) {
        object->_Node.Object().properties[prop.first] = std::make_shared<Node>(*prop.second);
    }

    // Check that the value matches type
    int type_size = type->_Node.Object().properties.size();

    if (type->_Node.Object().properties.count("_init")) {
        type_size--;
    }
    if (object->_Node.Object().properties.size() != type_size) {
        return throw_error("Error in object initialization for type '" + node->_Node.ObjectDeconstruct().name + "': Number of properties differs between object and type");
    }

    for (auto& prop : type->_Node.Object().properties) {
        if (prop.first == "_init") {
            continue;
        }

        std::string prop_name = prop.first;

        if (prop.second->TypeInfo.is_decl) {
            node_ptr symbol = get_symbol(prop.second->TypeInfo.type_name, current_scope);
            if (!symbol) {
                return throw_error("Type '" + prop.second->TypeInfo.type_name + "' is undefined");
            }
            prop.second = symbol;
        }

        node_ptr type_prop_value = prop.second;
        node_ptr obj_prop_value = object->_Node.Object().properties[prop_name];

        if (!obj_prop_value) {
            return throw_error("Error in object initialization for type '" + node->_Node.ObjectDeconstruct().name + "': Match error in property '" + prop_name + "': Missing property");
        }

        bool match = match_types(type_prop_value, obj_prop_value);

        if (!match) {
            return throw_error("Error in object initialization for type '" + node->_Node.ObjectDeconstruct().name + "': Match error in property '" + prop_name + "': Expected value of type '" + printable(type_prop_value) + "' but received value of type '" + printable(obj_prop_value) + "'");
        }

        object->_Node.Object().properties[prop_name]->Meta = type_prop_value->Meta;
        object->_Node.Object().properties[prop_name]->TypeInfo.type_name = type_prop_value->TypeInfo.type_name;
        object->_Node.Object().properties[prop_name]->TypeInfo.type = type_prop_value;
    }

    // Call the init function if it exists
    
    if (type->_Node.Object().properties.count("_init")) {
        node_ptr func = type->_Node.Object().properties["_init"];
        func->_Node.Function().closure["this"] = object;
        node_ptr func_call = new_node(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().name = "_init";
        tc_func_call(func_call, func);
    }

    // Evaluate onInit Hook

    if (type->Meta.onInitFunction) {

        node_ptr hook = copy_function(type->Meta.onInitFunction);
        hook = tc_function(hook);

        // Typecheck first param
        std::string param_name = hook->_Node.Function().params[0]->_Node.ID().value;
        node_ptr param_type = hook->_Node.Function().param_types[param_name];

        if (!param_type || param_type->type == NodeType::ANY) {
            hook->_Node.Function().param_types[param_name] = type;
        } else {
            if (!match_types(object, param_type)) {
                return throw_error("Hook's first paramater is expected to be of type '" + printable(type) + "' but was typed as '" + printable(param_type) + "'");
            }
        }

        node_ptr func_call = new_node(NodeType::FUNC_CALL);
        func_call->_Node.FunctionCall().args.push_back(object);
        tc_func_call(func_call, hook);
    }

    return object;
}

node_ptr Typechecker::copy_node(node_ptr& node, std::vector<node_ptr> bases) {

    if (!node) {
        return node;
    }

    for (node_ptr& elem : bases) {
        if (node == elem) {
            return node;
        }
    }

    bases.push_back(node);

    if (node->type == NodeType::LIST) {
        node_ptr list = new_node(NodeType::LIST);
        list->Meta = node->Meta;
        list->TypeInfo = node->TypeInfo;
        for (node_ptr elem : node->_Node.List().elements) {
            list->_Node.List().elements.push_back(copy_node(elem, bases));
        }
        return list;
    }

    if (node->type == NodeType::OBJECT) {
        node_ptr obj = new_node(NodeType::OBJECT);
        obj->Meta = node->Meta;
        obj->TypeInfo = node->TypeInfo;
        for (auto elem : node->_Node.Object().elements) {
            obj->_Node.Object().elements.push_back(copy_node(elem, bases));
        }
        for (auto prop : node->_Node.Object().properties) {
            obj->_Node.Object().properties[prop.first] = copy_node(prop.second, bases);
        }
        for (auto prop : node->_Node.Object().defaults) {
            obj->_Node.Object().defaults[prop.first] = copy_node(prop.second, bases);
        }
        for (auto key : node->_Node.Object().keys) {
            obj->_Node.Object().keys.push_back(key);
        }
        for (auto value : node->_Node.Object().values) {
            obj->_Node.Object().values.push_back(copy_node(value, bases));
        }
        return obj;
    }

    if (node->type == NodeType::FUNC) {
        return copy_function(node);
    }

    return std::make_shared<Node>(*node);
}