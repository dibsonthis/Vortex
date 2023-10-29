#pragma once
#include "../Typechecker.hpp"

std::string Typechecker::printable(node_ptr& node, std::vector<node_ptr> bases) {
    for (node_ptr& base : bases) {
        if (node == base) {
            return "...";
        }
    }
    switch (node->type) {
        case NodeType::NUMBER: {
            if (node->TypeInfo.is_literal_type) {}
            else if (node->TypeInfo.is_type) {
                return "Number";
            }
            std::string num_str = std::to_string(node->_Node.Number().value);
            num_str.erase(num_str.find_last_not_of('0') + 1, std::string::npos);
            num_str.erase(num_str.find_last_not_of('.') + 1, std::string::npos);
            return num_str;
        }
        case NodeType::BOOLEAN: {
            if (node->TypeInfo.is_literal_type) {}
            else if (node->TypeInfo.is_type) {
                return "Boolean";
            }
            return node->_Node.Boolean().value ? "true" : "false";
        }
        case NodeType::STRING: {
            if (node->TypeInfo.is_literal_type) {}
            else if (node->TypeInfo.is_type) {
                return "String";
            }
            return node->_Node.String().value;
        }
        case NodeType::FUNC: {
            if (node->TypeInfo.is_refinement_type && node->_Node.Function().name != "") {
                return node->_Node.Function().name;
            }
            std::string res = "(";
            for (int i = 0; i < node->_Node.Function().params.size(); i++) {
                node_ptr& param = node->_Node.Function().params[i];
                node_ptr& type = node->_Node.Function().param_types[param->_Node.ID().value];
                if (type) {
                    node_ptr _type = get_type(type);
                    res += param->_Node.ID().value + ": " + printable(_type, bases);
                } else {
                    res += param->_Node.ID().value;
                }
                if (i < node->_Node.Function().params.size()-1) {
                    res += ", ";
                }
            }
            if (node->_Node.Function().return_type) {
                res += ") => " + printable(node->_Node.Function().return_type, bases);
            } else {
                res += ") => ...";
            }
            if (node->_Node.Function().dispatch_functions.size() > 0) {
                res += "\n";
                for (auto& func : node->_Node.Function().dispatch_functions) {
                    res += printable(func, bases) += "\n";
                }
            }
            return res;
        }
        case NodeType::LIST: {
            bases.push_back(node);
            std::string res = "[";
            for (int i = 0; i < node->_Node.List().elements.size(); i++) {
                node_ptr value = node->_Node.List().elements[i];
                bool is_base = false;
                for (node_ptr& base : bases) {
                    if (value == base) {
                        if (base->type == NodeType::OBJECT) {
                            res += "{...}";
                        } else {
                            res += "[...]";
                        }
                        is_base = true;
                        break;
                    }
                }
                if (!is_base) {
                    res += printable(node->_Node.List().elements[i], bases);
                }
                if (i < node->_Node.List().elements.size()-1) {
                    res += ", ";
                }
            }
            res += "]";
            return res;
        }
        case NodeType::PIPE_LIST: {
            bases.push_back(node);
            std::string res = "";
            for (int i = 0; i < node->_Node.List().elements.size(); i++) {
                node_ptr value = node->_Node.List().elements[i];
                bool is_base = false;
                for (node_ptr& base : bases) {
                    if (value == base) {
                        if (base->type == NodeType::OBJECT) {
                            res += "{...}";
                        } else {
                            res += "[...]";
                        }
                        is_base = true;
                        break;
                    }
                }
                if (!is_base) {
                    res += printable(node->_Node.List().elements[i], bases);
                }
                if (i < node->_Node.List().elements.size()-1) {
                    res += " | ";
                }
            }
            res += "";
            return res;
        }
        case NodeType::OBJECT: {
            if (node->TypeInfo.is_general_type) {
                return "{}";
            }
            bases.push_back(node);
            std::string res = "";
            if (node->TypeInfo.type_name != "") {
                res += node->TypeInfo.type_name + " ";
            }
            res += "{ ";
            for (auto const& elem : node->_Node.Object().properties) {
                node_ptr value = elem.second;
                bool is_base = false;
                for (node_ptr& base : bases) {
                    if (value == base) {
                        if (base->type == NodeType::OBJECT) {
                            res += elem.first + ": {...} ";
                        } else {
                            res += elem.first + ": [...] ";
                        }
                        is_base = true;
                        break;
                    }
                }
                if (!is_base) {
                    res += elem.first + ": " + printable(value, bases) + ' ';
                }
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
        case NodeType::ANY: {
            return "Any";
        }
        case NodeType::NOVALUE: {
            return "NoValue";
        }
        case NodeType::_ERROR: {
            return "Error";
        }
        case NodeType::ID: {
            return node->_Node.ID().value;
        }
        case NodeType::OP: {
            if (node->_Node.Op().value == ".") {
                return printable(node->_Node.Op().left) + "." + printable(node->_Node.Op().right, bases);
            }
            return node->_Node.Op().value;
        }
        case NodeType::ACCESSOR: {
            return printable(node->_Node.Accessor().container) + printable(node->_Node.Accessor().accessor, bases);
        }
        default: {
            return "<not implemented>";
        }
    }
}