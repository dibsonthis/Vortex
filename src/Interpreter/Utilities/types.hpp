#pragma once
#include "../Interpreter.hpp"

node_ptr Interpreter::get_type(node_ptr& node, std::vector<node_ptr> bases = {}) {

    if (node == nullptr) {
        return new_node(NodeType::ANY);
    }

    if (node->type != NodeType::OBJECT && node->type != NodeType::LIST && node->TypeInfo.type) {
        return node->TypeInfo.type;
    }

    for (node_ptr& base : bases) {
        if (node == base) {
            return node;
        }
    }

    bases.push_back(node);

    if (node->type == NodeType::FUNC) {
        node_ptr func = std::make_shared<Node>(*node);
        func->_Node.Function().return_type = get_type(func->_Node.Function().return_type, bases);
        func->TypeInfo = node->TypeInfo;
        func->TypeInfo.is_type = true;
        return func;
    }

    if (node->type == NodeType::OBJECT) {
        node_ptr obj = new_node(NodeType::OBJECT);

        obj->TypeInfo = node->TypeInfo;

        for (auto& prop : node->_Node.Object().properties) {
            obj->_Node.Object().properties[prop.first] = get_type(prop.second, bases);
        }

        obj->TypeInfo.is_type = true;
        obj->TypeInfo.is_enum = node->TypeInfo.is_enum;

        return obj;
    }

    if (node->type != NodeType::LIST) {
        node_ptr res = std::make_shared<Node>(*node);
        res->TypeInfo.is_type = true;
        return res;
    }

    node_ptr types = new_node(NodeType::LIST);
    types->type = NodeType::PIPE_LIST;

    for (node_ptr& elem : node->_Node.List().elements) {

        bool match = false;

        node_ptr reduced = get_type(elem, bases);
        reduced->TypeInfo.is_type = true;

        if (reduced->type == NodeType::LIST) {
            sort(reduced->_Node.List().elements.begin(), reduced->_Node.List().elements.end(), 
            [this](node_ptr& a, node_ptr& b)
            { 
                return a->type < b->type;
            });
        }

        if (types->_Node.List().elements.size() == 0) {
            types->_Node.List().elements.push_back(reduced);
            continue;
        }
        for (node_ptr& t_elem : types->_Node.List().elements) {
            if (match_types(t_elem, reduced)) {
                match = true;
            }
        }

        if (!match) {
            types->_Node.List().elements.push_back(reduced);
        }
    }

    sort(types->_Node.List().elements.begin(), types->_Node.List().elements.end(), 
        [this](node_ptr& a, node_ptr& b)
        { 
            return a->type < b->type;
        });

    node_ptr list = new_node(NodeType::LIST);

    if (types->_Node.List().elements.size() == 0) {
        list->_Node.List().elements.push_back(new_node(NodeType::ANY));
    } else if (types->_Node.List().elements.size() == 1) {
        list->_Node.List().elements.push_back(types->_Node.List().elements[0]);
    } else {
        list->_Node.List().elements.push_back(types);
    }

    list->TypeInfo = node->TypeInfo;
    list->Meta = node->Meta;

    if (list->_Node.List().elements.size() == 0) {
        list->_Node.List().elements.push_back(new_node(NodeType::ANY));
    }

    return list;
}

bool Interpreter::match_types(node_ptr& _type, node_ptr& _value) {
    if (!_type || !_value) {
        return false;
    }

    if (_type == _value) {
        return true;
    }

    node_ptr type = std::make_shared<Node>(*_type);

    if (type->TypeInfo.type) {
        type = type->TypeInfo.type;
    }

    node_ptr value = std::make_shared<Node>(*_value);

    if (value->TypeInfo.type) {
        value = value->TypeInfo.type;
    }

    if (type == nullptr || value == nullptr) {
        return false;
    }

    if (type->type == NodeType::ANY || value->type == NodeType::ANY) {
        return true;
    }

    if (type == value) {
        return true;
    }

    // For forward declarations

    if (type->TypeInfo.is_decl) {
        node_ptr var = get_symbol(type->TypeInfo.type_name, current_scope);
        if (!var) {
            throw_error("Type '" + type->TypeInfo.type_name + "' is undefined");
            return false;
        }
        *_type = *var;
        type = var;
    }

    if (type->type == NodeType::ANY || value->type == NodeType::ANY) {
        return true;
    }

    // For refinement types
    
    if (type->type == NodeType::FUNC && type->TypeInfo.is_refinement_type) {
        node_ptr func_call = new_node(NodeType::FUNC_CALL);
        if (type->_Node.Function().params.size() != 1) {
            throw_error("Refinement type must have one parameter");
            return false;
        }
        node_ptr param = type->_Node.Function().params[0];
        node_ptr param_type = type->_Node.Function().param_types[param->_Node.ID().value];
        if (param_type && !match_types(param_type, value)) {
            return false;
        }

        func_call->_Node.FunctionCall().name = type->_Node.Function().name;
        func_call->_Node.FunctionCall().args.push_back(value);
        node_ptr res = eval_func_call(func_call, type);
        if (res->type == NodeType::ERROR) {
            return false;
        }
        if (res->type != NodeType::BOOLEAN) {
            throw_error("Refinement types must return a boolean value");
            return false;
        }
        return res->_Node.Boolean().value;
    }

    // For literal types
    
    if (type->TypeInfo.is_literal_type) {
        return match_values(type, value);
    }

    // If pipe list: String | Number

    if (type->type == NodeType::PIPE_LIST && value->type == NodeType::PIPE_LIST) {
        // Check that all elements in value
        // Are in the type union
        for (node_ptr& elem : value->_Node.List().elements) {
            if (!match_types(type, elem)) {
                return false;
            }
        }

        return true;
    }

    if (type->type == NodeType::PIPE_LIST) {
        for (node_ptr& elem : type->_Node.List().elements) {
            if (match_types(elem, value)) {
                return true;
            }
        }

        return false;
    }

    // If union list: union _ {String, Number}

    if (type->type == NodeType::LIST && type->TypeInfo.is_union && value->type == NodeType::LIST && value->TypeInfo.is_union) {
        // Check that all elements in value
        // Are in the type union
        for (node_ptr& elem : value->_Node.List().elements) {
            if (!match_types(type, elem)) {
                return false;
            }
        }

        return true;
    }

    if (type->type == NodeType::LIST && type->TypeInfo.is_union) {
        for (node_ptr& elem : type->_Node.List().elements) {
            if (match_types(elem, value)) {
                return true;
            }
        }

        return false;
    }

    // From here on out, we assume they are the same type

    if (type->type != value->type) {
        return false;
    }

    // For Function types
    
    if (type->type == NodeType::FUNC) {
        if (type->TypeInfo.is_general_type) {
            return true;
        }

        if (value->TypeInfo.is_general_type) {
            return true;
        }

        // Match return types

        if (!type->_Node.Function().return_type) {
            // Assuming () => Type
            type->_Node.Function().return_type = eval_node(type->_Node.Function().body);
        }

        if (type->_Node.Function().return_type && value->_Node.Function().return_type) {
            if (!match_types(type->_Node.Function().return_type, value->_Node.Function().return_type)) {
                return false;
            }
        }

        auto& type_params = type->_Node.Function().params;
        auto& value_params = value->_Node.Function().params;

        if (type_params.size() != value_params.size()) {
            return false;
        }

        // We check the param types, but we don't care if they are
        // named the same, we just care about positional params

        for (int i = 0; i < type_params.size(); i++) {
            std::string type_param = type_params[i]->_Node.ID().value;
            std::string value_param = value_params[i]->_Node.ID().value;;

            node_ptr type_param_type = type->_Node.Function().param_types[type_param];
            node_ptr value_param_type = value->_Node.Function().param_types[value_param];

            if (type_param_type && value_param_type) {
                if (!match_types(type_param_type, value_param_type)) {
                    return false;
                }
            }
        }

        return true;
    }

    if (type->type == NodeType::LIST) {
        // If [] or List
        if (type->_Node.List().elements.size() == 0 && type->TypeInfo.is_type) {
            return true;
        }

        node_ptr left = get_type(type);
        node_ptr right = get_type(value);

        if (left->_Node.List().elements.size() != right->_Node.List().elements.size()) {
            return false;
        }

        for (int i = 0; i < left->_Node.List().elements.size(); i++) {
            if (!match_types(left->_Node.List().elements[i], right->_Node.List().elements[i])) {
                return false;
            }
        }

        return true;
    }

    if (type->type == NodeType::OBJECT) {
        if (type->TypeInfo.is_general_type) {
            return true;
        }

        if (value->TypeInfo.is_general_type) {
            return true;
        }
        
        if (type->TypeInfo.type_name != value->TypeInfo.type_name) {
            return false;
        }

        if (type->TypeInfo.type_name != "" && value->TypeInfo.type_name != "" && type->TypeInfo.type_name == value->TypeInfo.type_name) {
            // If they both have the same type name
            // They must be the same type
            // Because instantiation of types won't allow an object
            // to have a type without adhering to its properties
            return true;
        }

        // If 'Object'
        if (type->TypeInfo.is_general_type && value->TypeInfo.type_name == "") {
            return true;
        }

        // Remove references to _init() here

        if (type->_Node.Object().properties.count("_init")) {
            type->_Node.Object().properties.erase("_init");
        }

        if (value->_Node.Object().properties.count("_init")) {
            value->_Node.Object().properties.erase("_init");
        }

        if (type->_Node.Object().properties.size() != value->_Node.Object().properties.size()) {
            return false;
        }

        for (auto& prop : type->_Node.Object().properties) {
            std::string prop_name = prop.first;

            if (!value->_Node.Object().properties.count(prop_name)) {
                return false;
            }

            node_ptr a_prop_value = prop.second;
            node_ptr b_prop_value = value->_Node.Object().properties[prop_name];

            int match = match_types(a_prop_value, b_prop_value);
            if (!match) {
                return false;
            }
        }

        return true;
    }

    if (type->type == value->type) {
        return true;
    }

    return false;
}

bool Interpreter::match_values(node_ptr& nodeA, node_ptr& nodeB) {
    if (!nodeA || !nodeB) {
        return false;
    }

    if (nodeA->type == NodeType::ANY || nodeB->type == NodeType::ANY) {
        return true;
    }

    if (nodeA->type != nodeB->type) {
        return false;
    }

    if (nodeA->type == NodeType::NONE) {
        return true;
    }

    if (nodeA == nodeB) {
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
            if (!nodeB->_Node.Object().properties.count(prop.first)) {
                return false;
            }
            if (!match_values(nodeA->_Node.Object().properties[prop.first], nodeB->_Node.Object().properties[prop.first])) {
                return false;
            }
        }
        return true;
    }
    if (nodeA->type == NodeType::PIPE_LIST) {
        if (nodeA->_Node.List().elements.size() != nodeB->_Node.List().elements.size()) {
            return false;
        }

        for (int i = 0; i < nodeA->_Node.List().elements.size(); i++) {
            if (!match_types(nodeA->_Node.List().elements[i], nodeB->_Node.List().elements[i])) {
                return false;
            }
        }
        return true;
    }

    if (nodeA->type == NodeType::ANY || nodeA->type == NodeType::NONE) {
        return true;
    }

    return false;
}

bool Interpreter::compareNodeTypes(node_ptr& lhs, node_ptr& rhs) {
    if (lhs->TypeInfo.is_literal_type && rhs->TypeInfo.is_literal_type) {
        if (match_types(lhs, rhs)) {
            return match_values(lhs, rhs);
        }
    }
    return match_types(lhs, rhs);
}