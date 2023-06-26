#include "Node.hpp"

std::string node_repr(node_ptr node) {
    switch (node->type) {
        case NodeType::ID: {
            return node->_Node.ID().value;
        }
        case NodeType::NUMBER: {
            return "Number";
        }
        case NodeType::STRING: {
            return "String";
        }
        case NodeType::BOOLEAN: {
            return "Boolean";
        }
        case NodeType::LIST: {
            if (node->TypeInfo.is_union && node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            if (node->_Node.List().elements.size() == 0) {
                return "List";
            } else if (node->_Node.List().elements.size() == 1) {
                return "[" + node_repr(node->_Node.List().elements[0]) + "]";
            } else if (node->_Node.List().elements.size() > 10) {
                std::string res = "[";
                for (int i = 0; i < 10; i++) {
                    res += node_repr(node->_Node.List().elements[i]);
                    if (i != 9) {
                        res += ", ";
                    }
                }
                res += " ... ]";
                return res;
            }
            std::string res = "[";
            for (int i = 0; i < node->_Node.List().elements.size(); i++) {
                res += node_repr(node->_Node.List().elements[i]);
                if (i < node->_Node.List().elements.size()-1) {
                    res += ", ";
                }
            }
            res += "]";
            return res;
        }
        case NodeType::PIPE_LIST: {
            if (node->TypeInfo.is_union && node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            if (node->_Node.List().elements.size() == 1) {
                return node_repr(node->_Node.List().elements[0]);
            } else if (node->_Node.List().elements.size() > 10) {
                std::string res = "";
                for (int i = 0; i < 10; i++) {
                    res += node_repr(node->_Node.List().elements[i]);
                    if (i != 9) {
                        res += " | ";
                    }
                }
                res += " ...";
                return res;
            }
            std::string res = "";
            for (int i = 0; i < node->_Node.List().elements.size(); i++) {
                res += node_repr(node->_Node.List().elements[i]);
                if (i < node->_Node.List().elements.size()-1) {
                    res += " | ";
                }
            }
            res += "";
            return res;
        }
        case NodeType::OBJECT: {
            if (node->TypeInfo.type && node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            if (node->TypeInfo.is_type && node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            if (node->TypeInfo.is_enum && node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            if (node->TypeInfo.type_name != "") {
                return node->TypeInfo.type_name;
            }
            return "Object";
        }
        case NodeType::LIB: {
            return "Library";
        }
        case NodeType::START_OF_FILE: {
            return "SOF";
        }
        case NodeType::END_OF_FILE: {
            return "EOF";
        }
        case NodeType::CONSTANT_DECLARATION: {
            return "Const declaration";
        }
        case NodeType::VARIABLE_DECLARATION: {
            return "Variable declaration";
        }
        case NodeType::FUNC_CALL: {
            return "Function call";
        }
        case NodeType::FUNC: {
            if (node->_Node.Function().name != "" && node->TypeInfo.is_refinement_type) {
                return node->_Node.Function().name;
            } else if (node->TypeInfo.is_type && node->_Node.Function().name != "") {
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
                if (node->_Node.Function().body) {
                    res += ") => " + node_repr(node->_Node.Function().body);
                } else {
                    res += ") => ...";
                }
                if (node->_Node.Function().dispatch_functions.size() > 0) {
                    res += "\n";
                    for (auto& func : node->_Node.Function().dispatch_functions) {
                        res += node_repr(func) += "\n";
                    }
                }
                return res;
            }
            
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
                    res += node_repr(func) += "\n";
                }
            }
            return res;
        }
        case NodeType::IMPORT: {
            return "Import statement";
        }
        case NodeType::POINTER: {
            return "Pointer";
        }
        case NodeType::NONE: {
            return "None";
        }
        case NodeType::ANY: {
            return "Any";
        }
        case NodeType::ERROR: {
            return "Error";
        }
        default: {
            return "<not_implemented>";
        }
    }
}