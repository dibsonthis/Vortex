#pragma once
#include "../Typechecker.hpp"
#include "types.hpp"

node_ptr Typechecker::tc_pos_neg(node_ptr& node) {
    node_ptr value = tc_node(node->_Node.Op().right);
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

node_ptr Typechecker::tc_not(node_ptr& node) {
    node_ptr value = tc_node(node->_Node.Op().right);

    if (value->type == NodeType::ERROR) {
        return throw_error(value->_Node.Error().message);
    }

    if (value->type == NodeType::BOOLEAN) {
        return new_boolean_node(!value->_Node.Boolean().value);
    }

     return new_boolean_node(value->type == NodeType::NONE);
}

node_ptr Typechecker::tc_add(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

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

node_ptr Typechecker::tc_sub(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->_Node.Number().value - right->_Node.Number().value);
    }

    if (left->type == NodeType::PIPE_LIST && right->type == NodeType::PIPE_LIST) {
        node_ptr union_list = new_node(NodeType::LIST);
        union_list->type = NodeType::PIPE_LIST;

        for (node_ptr& elem : left->_Node.List().elements) {
            if (match_types(right, elem)) {
                continue;
            }

            union_list->_Node.List().elements.push_back(elem);
        }

        return union_list;
    }

    if (left->type == NodeType::PIPE_LIST) {
        node_ptr union_list = new_node(NodeType::LIST);
        union_list->type = NodeType::PIPE_LIST;

        for (node_ptr& elem : left->_Node.List().elements) {
            if (match_types(elem, right)) {
                continue;
            }

            union_list->_Node.List().elements.push_back(elem);
        }

        return union_list;
    }

    return throw_error("Cannot perform operation '-' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_mul(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(left->_Node.Number().value * right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '*' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_div(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        if (right->_Node.Number().value == 0) {
            node_ptr res = new_number_node(0);
            res->TypeInfo.is_type = true;
            return res;
        }
        return new_number_node(left->_Node.Number().value / right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '/' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_pow(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(pow(left->_Node.Number().value, right->_Node.Number().value));
    }

    return throw_error("Cannot perform operation '^' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_mod(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node(fmod(left->_Node.Number().value, right->_Node.Number().value));
    }

    return throw_error("Cannot perform operation '^' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_eq_eq(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::BOOLEAN);
    }

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

    if (left->type == NodeType::LIST) {
        return new_boolean_node(match_values(left, right));
    }

    if (left->type == NodeType::OBJECT) {
        return new_boolean_node(match_values(left, right));
    }

    return new_boolean_node(false);
}

node_ptr Typechecker::tc_not_eq(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::BOOLEAN);
    }

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

    if (left->type == NodeType::LIST) {
        return new_boolean_node(!match_values(left, right));
    }

    if (left->type == NodeType::OBJECT) {
        return new_boolean_node(!match_values(left, right));
    }

    return new_boolean_node(true);
}

node_ptr Typechecker::tc_lt_eq(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::BOOLEAN);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value <= right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '<=' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_gt_eq(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::BOOLEAN);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value >= right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '>=' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_lt(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::BOOLEAN);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value < right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '<' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_gt(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::BOOLEAN);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_boolean_node(left->_Node.Number().value > right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '>' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_and(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (left->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    node_ptr left_bool = left;

    if (left->type != NodeType::BOOLEAN) {
        left_bool = new_boolean_node(left->type != NodeType::NONE);
    }

    if (!left_bool->_Node.Boolean().value) {
        return left;
    }

    node_ptr right = tc_node(node->_Node.Op().right);

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    return right;
}

node_ptr Typechecker::tc_bit_and(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node((long)left->_Node.Number().value & (long)right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '&' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_bit_or(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        return new_number_node((long)left->_Node.Number().value | (long)right->_Node.Number().value);
    }

    return throw_error("Cannot perform operation '|' on types: " + node_repr(left) + ", " + node_repr(right));
    return new_node(NodeType::NONE);
}

node_ptr Typechecker::tc_or(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr left_bool = left;

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (left->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type != NodeType::BOOLEAN) {
        left_bool = new_boolean_node(left->type != NodeType::NONE);
    }

    if (left_bool->_Node.Boolean().value) {
        return left;
    }

    node_ptr right = tc_node(node->_Node.Op().right);

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    return right;
}

node_ptr Typechecker::tc_null_op(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (left->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    if (left->type != NodeType::NONE) {
        return left;
    }

    node_ptr right = tc_node(node->_Node.Op().right);

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (right->type == NodeType::ANY) {
        return new_node(NodeType::ANY);
    }

    return right;
}

node_ptr Typechecker::tc_plus_eq(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    node_ptr plus_node = new_node(NodeType::OP);
    plus_node->_Node.Op().value = "+";
    plus_node->_Node.Op().left = left;
    plus_node->_Node.Op().right = right;
    plus_node = tc_add(plus_node);

    node_ptr eq_node = new_node(NodeType::OP);
    eq_node->_Node.Op().value = "=";
    eq_node->_Node.Op().left = node->_Node.Op().left;
    eq_node->_Node.Op().right = plus_node;
    eq_node = tc_eq(eq_node);

    return eq_node;
}

node_ptr Typechecker::tc_minus_eq(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);
    node_ptr right = tc_node(node->_Node.Op().right);

    node_ptr minus_node = new_node(NodeType::OP);
    minus_node->_Node.Op().value = "-";
    minus_node->_Node.Op().left = left;
    minus_node->_Node.Op().right = right;
    minus_node = tc_sub(minus_node);

    node_ptr eq_node = new_node(NodeType::OP);
    eq_node->_Node.Op().value = "=";
    eq_node->_Node.Op().left = node->_Node.Op().left;
    eq_node->_Node.Op().right = minus_node;
    eq_node = tc_eq(eq_node);

    return eq_node;
}

node_ptr Typechecker::tc_as(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    if (node->_Node.Op().right->type == NodeType::ID && node->_Node.Op().right->_Node.ID().value == "Const") {
        node_ptr copy = std::make_shared<Node>(*left);
        copy->Meta.is_const = true;
        return copy;
    }

    if (node->_Node.Op().right->type == NodeType::ID && node->_Node.Op().right->_Node.ID().value == "Type") {
        node_ptr copy = std::make_shared<Node>(*left);
        copy->TypeInfo.is_type = true;
        return copy;
    }

    if (node->_Node.Op().right->type == NodeType::ID && node->_Node.Op().right->_Node.ID().value == "Literal") {
        node_ptr copy = std::make_shared<Node>(*left);
        copy->TypeInfo.is_type = true;
        if (copy->type == NodeType::LIST) {
            copy->TypeInfo.is_tuple = true;
        } else {
            copy->TypeInfo.is_literal_type = true;
        }
        return copy;
    }

    if (node->_Node.Op().right->type == NodeType::ID && node->_Node.Op().right->_Node.ID().value == "Refinement") {
        node_ptr copy = std::make_shared<Node>(*left);
        copy->TypeInfo.is_type = true;
        copy->TypeInfo.is_refinement_type = true;
        return copy;
    }

    if (node->_Node.Op().right->type == NodeType::ID && node->_Node.Op().right->_Node.ID().value == "Union") {
        if (left->type == NodeType::LIST) {
            node_ptr union_list = new_node(NodeType::LIST);
            union_list->type = NodeType::PIPE_LIST;
            for (node_ptr& elem : left->_Node.List().elements) {
                node_ptr elem_copy = std::make_shared<Node>(*elem);
                if (!elem_copy->TypeInfo.is_type) {
                    if (elem_copy->type != NodeType::OBJECT && elem_copy->type != NodeType::LIST) {
                        elem_copy->TypeInfo.is_literal_type = true;
                    }
                }
                if (elem_copy->type == NodeType::PIPE_LIST) {
                    for (node_ptr e: elem_copy->_Node.List().elements) {
                        union_list->_Node.List().elements.push_back(e);
                    }
                } else {
                    union_list->_Node.List().elements.push_back(elem_copy);
                }
            }
            union_list->_Node.List().elements = sort_and_unique(union_list->_Node.List().elements);
            return union_list;
        }

        return left;
    }

    if (node->_Node.Op().right->type == NodeType::ID && node->_Node.Op().right->_Node.ID().value == "Iterator") {
        if (left->type == NodeType::PIPE_LIST) {
            node_ptr list = new_node(NodeType::LIST);
            for (node_ptr& e : left->_Node.List().elements) {
                node_ptr clone = copy_node(e);
                clone->TypeInfo.is_literal_type = false;
                list->_Node.List().elements.push_back(clone);
            }
            return list;
        }
        
        node_ptr list = new_node(NodeType::LIST);
        list->_Node.List().elements.push_back(left);
        return list;
    }

    node_ptr right = tc_node(node->_Node.Op().right);

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    // During typechecking
    if (left->type == NodeType::PIPE_LIST) {
        // Check to see if type we're casting to is contained inside the union
        bool match = match_types(left, right);
        if (match) {
            return right;
        }

        return throw_error("Cannot cast from '" + printable(left) + "' to '" + printable(right) + "'");
    }

    if (left->type == NodeType::OBJECT && right->type == NodeType::OBJECT) {
        node_ptr copy_left = std::make_shared<Node>(*left);
        node_ptr copy_right = std::make_shared<Node>(*right);
        
        copy_left->TypeInfo.type_name = "";
        copy_right->TypeInfo.type_name = "";

        if (match_types(copy_left, copy_right)) {
            copy_left->TypeInfo.type_name = right->TypeInfo.type_name;
            return copy_left;
        }

        return throw_error("Cannot cast from '" + printable(left) + "' to '" + printable(right) + "'");
    }

    if (left->type == right->type) {
        return left;
    }

    if (left->type == NodeType::ANY) {
        return right;
    }

    return throw_error("Cannot perform operation 'as' on types: " + printable(left) + ", " + printable(right));
}

node_ptr Typechecker::tc_is(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    node_ptr right = tc_node(node->_Node.Op().right);

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (match_types(left, right)) {
        if (node->_Node.Op().left->type == NodeType::ID) {
            node_ptr value = std::make_shared<Node>(*right);
            add_symbol(node->_Node.Op().left->_Node.ID().value, value, current_scope);
        } else {
            current_scope->cast_types[left] = copy_node(left);
            *left = *right;
        }

        return new_boolean_node(true);
    }

    return new_boolean_node(false);
}

node_ptr Typechecker::tc_in(node_ptr& node) {
    node_ptr left = tc_node(node->_Node.Op().left);

    if (left->type == NodeType::ERROR) {
        return throw_error(left->_Node.Error().message);
    }

    node_ptr right = tc_node(node->_Node.Op().right);

    if (right->type == NodeType::ERROR) {
        return throw_error(right->_Node.Error().message);
    }

    if (right->type != NodeType::LIST && right->type != NodeType::PIPE_LIST ) {
        return new_boolean_node(match_types(right, left) && match_values(right, left));
    }

    for (node_ptr& elem : right->_Node.List().elements) {
        if (match_types(elem, left)) {
            return new_boolean_node(true);
        }
    }

    return new_boolean_node(false);
}