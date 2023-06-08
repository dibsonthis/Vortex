#pragma once
#include "Parser.hpp"

void Parser::advance(int n) {
    index += n;
    if (index < nodes.size()) {
        current_node = nodes[index];
        line = current_node->line;
        column = current_node->column;
    }
}

node_ptr Parser::peek(int n) {
    int idx = index + n;
    if (idx < nodes.size()) {
        return nodes[idx];
    }
    return nodes[nodes.size()-1];
}

void Parser::reset(int idx) {
    index = idx;
    current_node = nodes[index];
}

void Parser::parse_comma(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && !has_children(current_node) && current_node->_Node.Op().value == ",") {
            node_ptr left = peek(-1);
            node_ptr right = peek(1);

            if ((left->type == NodeType::OP && !has_children(left)) || (right->type == NodeType::OP && !has_children(right))) {
                erase_curr();
                continue;
            }

            current_node->_Node.Op().left = left;
            current_node->_Node.Op().right = right;
            erase_next();
            erase_prev();
        }
        advance();
    }
}

void Parser::parse_bin_op(std::vector<std::string> operators, std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && !has_children(current_node) && vector_contains_string(operators, current_node->_Node.Op().value)) {
            node_ptr left = peek(-1);
            node_ptr right = peek(1);
            current_node->_Node.Op().left = left;
            current_node->_Node.Op().right = right;
            erase_next();
            erase_prev();
        }
        advance();
    }
}

void Parser::parse_un_op_amb(std::vector<std::string> operators, std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        node_ptr prev = peek(-1);
        if (current_node->type == NodeType::OP && vector_contains_string(operators, current_node->_Node.Op().value) && 
            (
                (prev->type == NodeType::OP && !has_children(prev)) ||
                prev->type == NodeType::START_OF_FILE ||
                prev->type == NodeType::PAREN && prev->_Node.Paren().elements.size() == 0 ||
                prev->type == NodeType::LIST && prev->_Node.List().elements.size() == 0
            )) {
            node_ptr right = peek(1);
            current_node->_Node.Op().right = right;
            erase_next();

            if (current_node->_Node.Op().value == "&") {
                if (right->type != NodeType::ID) {
                    error_and_exit("Cannot capture a reference of a literal - '&' must be followed by a variable name");
                }
                current_node->type = NodeType::REF;
                current_node->_Node = RefNode();
                current_node->_Node.Ref().value = right;
            }
        }
        advance();
    }
}

void Parser::parse_un_op(std::vector<std::string> operators, std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && vector_contains_string(operators, current_node->_Node.Op().value)) {
            node_ptr right = peek(1);
            current_node->_Node.Op().right = right;
            erase_next();
        }
        advance();
    }
}

void Parser::parse_post_op(std::vector<std::string> operators, std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && vector_contains_string(operators, current_node->_Node.Op().value) && 
            (
                peek(1)->type == NodeType::OP ||
                peek(1)->type == NodeType::END_OF_FILE
            )) {
            node_ptr left = peek(-1);
            current_node->_Node.Op().right = left;
            erase_prev();
        }
        advance();
    }
}

void Parser::parse_dot(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && !has_children(current_node) && current_node->_Node.Op().value == ".") {
            node_ptr left = peek(-1);
            node_ptr right = peek(1);

            if (right->type == NodeType::ACCESSOR) {
                current_node->_Node.Op().left = left;
                current_node->_Node.Op().right = right->_Node.Accessor().container;
                right->_Node.Accessor().container = current_node;
                nodes[index] = right;
            } else {
                current_node->_Node.Op().left = left;
                current_node->_Node.Op().right = right;
            }
            erase_next();
            erase_prev();
        }
        advance();
    }
}

void Parser::parse_equals(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && !has_children(current_node) && current_node->_Node.Op().value == "=") {
            node_ptr left = peek(-1);
            node_ptr right = peek(1);
            current_node->_Node.Op().left = left;
            current_node->_Node.Op().right = right;
            erase_next();
            erase_prev();

            if (current_node->_Node.Op().right->type == NodeType::FUNC) {
                if (left->type == NodeType::ID) {
                    current_node->_Node.Op().right->_Node.Function().name = left->_Node.ID().value;
                } else if (left->type == NodeType::OP && left->_Node.Op().value == ".") {
                    current_node->_Node.Op().right->_Node.Function().name = left->_Node.Op().right->_Node.ID().value;
                }
            }
        }
        advance();
    }
}

void Parser::parse_colon(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && !has_children(current_node) && current_node->_Node.Op().value == ":") {
            node_ptr left = peek(-1);
            node_ptr right = peek(1);
            current_node->_Node.Op().left = left;
            current_node->_Node.Op().right = right;
            erase_next();
            erase_prev();

            if (current_node->_Node.Op().right->type == NodeType::FUNC) {
                current_node->_Node.Op().right->_Node.Function().name = left->_Node.ID().value;
            }
        }
        advance();
    }
}

void Parser::parse_list(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == "[") {
            current_node->type = NodeType::LIST;
            current_node->_Node = ListNode();
            int curr_idx = index;
            int closing_index = find_closing_index(index, "[", "]");
            advance();
            parse(index, "]");
            advance(-1);
            while (true) {
                if (peek()->type == NodeType::END_OF_FILE) {
                    error_and_exit("Missing end ']'");
                }
                if (peek()->type == NodeType::OP && peek()->_Node.Op().value == "]") {
                    break;
                }
                nodes[curr_idx]->_Node.List().elements.push_back(peek());
                erase_next();
            }
            erase_next();
        }

        advance();
    }
}

void Parser::parse_object(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == "{") {
            current_node->type = NodeType::OBJECT;
            current_node->_Node = ObjectNode();
            int curr_idx = index;
            int closing_index = find_closing_index(index, "{", "}");
            advance();
            parse(index, "}");
            advance(-1);
            while (true) {
                if (peek()->type == NodeType::END_OF_FILE) {
                    error_and_exit("Missing end '}'");
                }
                if (peek()->type == NodeType::OP && peek()->_Node.Op().value == "}") {
                    break;
                }
                nodes[curr_idx]->_Node.Object().elements.push_back(peek());
                erase_next();
            }
            erase_next();
        }

        advance();
    }
}

void Parser::parse_enum(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "enum" && peek()->type == NodeType::ID && peek(2)->type == NodeType::OBJECT) {
            current_node->type = NodeType::ENUM;
            current_node->_Node = EnumNode();
            current_node->_Node.Enum().name = peek()->_Node.ID().value;
            current_node->_Node.Enum().body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_union(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "union" && peek()->type == NodeType::ID && peek(2)->type == NodeType::OBJECT) {
            current_node->type = NodeType::UNION;
            current_node->_Node = UnionNode();
            current_node->_Node.Union().name = peek()->_Node.ID().value;
            current_node->_Node.Union().body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_type(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "type" && (peek()->type == NodeType::OBJECT_DECONSTRUCT)) {
            current_node->type = NodeType::TYPE;
            current_node->_Node = TypeNode();
            current_node->_Node.Type().name = peek()->_Node.ObjectDeconstruct().name;
            current_node->_Node.Type().body = peek()->_Node.ObjectDeconstruct().body;
            erase_next();
        } else if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "type" && (peek()->type == NodeType::ID || peek()->type == NodeType::ACCESSOR) && peek(2)->type == NodeType::OP && peek(2)->_Node.Op().value == "=") {
            current_node->type = NodeType::TYPE;
            current_node->_Node = TypeNode();
            current_node->_Node.Type().name = peek()->_Node.ID().value;
            current_node->_Node.Type().expr = peek(3);
            current_node->_Node.Type().expr->TypeInfo.is_type = true;
            if (current_node->_Node.Type().expr->type == NodeType::FUNC) {
                current_node->_Node.Type().expr->_Node.Function().name = current_node->_Node.Type().name;
            }
            erase_next();
            erase_next();
            erase_next();
        } else if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "type" && (peek()->type == NodeType::ID)) {
            current_node->type = NodeType::TYPE;
            current_node->_Node = TypeNode();
            current_node->_Node.Type().name = peek()->_Node.ID().value;
            current_node->_Node.Type().body = nullptr;
            current_node->TypeInfo.is_decl = true;
        }
        advance();
    }
}

// void Parser::parse_type_ext(std::string end) {
//     while (current_node->type != NodeType::END_OF_FILE) {
//         if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
//             break;
//         }
//         if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "extend" && peek(2)->type == NodeType::OBJECT) {
//             current_node->type = NodeType::TYPE_EXT;
//             current_node->_Node = TypeExtNode();
//             current_node->_Node.TypeExt().type = peek();
//             current_node->_Node.TypeExt().body = peek(2);
//             erase_next();
//             erase_next();
//         }
//         advance();
//     }
// }

void Parser::parse_hook_implementation(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == "::" && peek(-1)->type == NodeType::ID && peek()->type == NodeType::FUNC) {
            current_node->type = NodeType::HOOK;
            current_node->_Node = HookNode();
            current_node->_Node.Op().value = "";
            current_node->_Node.Hook().hook_name = peek(-1)->_Node.ID().value;
            current_node->_Node.Hook().function = peek(1);
            erase_next();
            erase_prev();
        }
        advance();
    }
}

void Parser::parse_func_def(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == "=>" && peek()->type != NodeType::END_OF_FILE) {
            if (peek(-1)->type == NodeType::PAREN)
            {
                current_node->type = NodeType::FUNC;
                current_node->_Node = FuncNode();
                node_ptr params = peek(-1);
                if (params->_Node.Paren().elements.size() == 1 && params->_Node.Paren().elements[0]->type == NodeType::COMMA_LIST) {
                    for (node_ptr& elem : params->_Node.Paren().elements[0]->_Node.List().elements) {
                        current_node->_Node.Function().params.push_back(elem);
                        current_node->_Node.Function().args.push_back(nullptr);
                    }
                } else if (params->_Node.Paren().elements.size() == 1) {
                    current_node->_Node.Function().params.push_back(params->_Node.Paren().elements[0]);
                    current_node->_Node.Function().args.push_back(nullptr);
                }
                current_node->_Node.Function().body = peek();
                erase_next();
                erase_prev();
            } else if ( 
                peek(-2)->type == NodeType::OP && peek(-2)->_Node.Op().value == ":" && 
                peek(-3)->type == NodeType::PAREN)
            {
                current_node->type = NodeType::FUNC;
                current_node->_Node = FuncNode();
                node_ptr return_type = peek(-1);
                node_ptr params = peek(-3);

                if (params->_Node.Paren().elements.size() == 1 && params->_Node.Paren().elements[0]->type == NodeType::COMMA_LIST) {
                    for (node_ptr& elem : params->_Node.Paren().elements[0]->_Node.List().elements) {
                        current_node->_Node.Function().params.push_back(elem);
                        current_node->_Node.Function().args.push_back(nullptr);
                    }
                } else if (params->_Node.Paren().elements.size() == 1) {
                    current_node->_Node.Function().params.push_back(params->_Node.Paren().elements[0]);
                    current_node->_Node.Function().args.push_back(nullptr);
                }
                current_node->_Node.Function().body = peek();
                current_node->_Node.Function().return_type = return_type;
                erase_next();
                erase_prev();
                erase_prev();
                erase_prev();
            }

            if (current_node->type == NodeType::FUNC && current_node->_Node.Function().body->type == NodeType::OBJECT) {
                for (int i = 0; i < current_node->_Node.Function().body->_Node.Object().elements.size(); i++) {
                    auto& elem = current_node->_Node.Function().body->_Node.Object().elements[i];
                    if (elem->type == NodeType::OP && elem->_Node.Op().value == ";") {
                        current_node->_Node.Function().body->_Node.Object().elements.erase(current_node->_Node.Function().body->_Node.Object().elements.begin() + i);
                    }
                }
            }
        }
        advance();
    }
}

void Parser::parse_paren(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == "(") {
            current_node->type = NodeType::PAREN;
            current_node->_Node = ParenNode();
            int curr_idx = index;
            int closing_index = find_closing_index(index, "(", ")");
            advance();
            parse(index, ")");
            advance(-1);
            while (true) {
                if (peek()->type == NodeType::END_OF_FILE) {
                    error_and_exit("Missing end ')'");
                }
                if (peek()->type == NodeType::OP && peek()->_Node.Op().value == ")") {
                    break;
                }
                nodes[curr_idx]->_Node.Paren().elements.push_back(peek());
                erase_next();
            }
            erase_next();
        }

        advance();
    }
}

void Parser::parse_func_call(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "ret") {
            advance();
            continue;
        }
        if (current_node->type == NodeType::ID && peek()->type == NodeType::PAREN) {
            std::string name = current_node->_Node.ID().value;
            current_node->type = NodeType::FUNC_CALL;
            current_node->_Node = FuncCallNode();
            current_node->_Node.FunctionCall().name = name;
            node_ptr argsList = peek();
            if (argsList->_Node.Paren().elements.size() == 1 && argsList->_Node.Paren().elements[0]->type == NodeType::COMMA_LIST) {
                current_node->_Node.FunctionCall().args = argsList->_Node.Paren().elements[0]->_Node.List().elements;
            } else if (argsList->_Node.Paren().elements.size() == 1) {
                current_node->_Node.FunctionCall().args.push_back(argsList->_Node.Paren().elements[0]);
            }
            erase_next();
        } else if (current_node->type == NodeType::PAREN && current_node->_Node.Paren().elements.size() == 1 && peek()->type == NodeType::PAREN) {
            node_ptr func = current_node->_Node.Paren().elements[0];
            current_node->type = NodeType::FUNC_CALL;
            current_node->_Node = FuncCallNode();
            current_node->_Node.FunctionCall().inline_func = func;
            node_ptr argsList = peek();
            if (argsList->_Node.Paren().elements.size() == 1 && argsList->_Node.Paren().elements[0]->type == NodeType::COMMA_LIST) {
                current_node->_Node.FunctionCall().args = argsList->_Node.Paren().elements[0]->_Node.List().elements;
            } else if (argsList->_Node.Paren().elements.size() == 1) {
                current_node->_Node.FunctionCall().args.push_back(argsList->_Node.Paren().elements[0]);
            }
            erase_next();
        }
        advance();
    }
}

void Parser::parse_object_desconstruct(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "ret") {
            advance();
            continue;
        }
        if (current_node->type == NodeType::ID && peek()->type == NodeType::OBJECT) {
            std::string name = current_node->_Node.ID().value;
            current_node->type = NodeType::OBJECT_DECONSTRUCT;
            current_node->_Node = ObjectDeconstructNode();
            current_node->_Node.ObjectDeconstruct().name = name;
            current_node->_Node.ObjectDeconstruct().body = peek();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_accessor(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "import") {
            advance();
            continue;
        }
        while ((current_node->type == NodeType::ID || current_node->type == NodeType::LIST || current_node->type == NodeType::FUNC_CALL || current_node->type == NodeType::ACCESSOR || (current_node->type == NodeType::OP && current_node->_Node.Op().value == ".") || 
        current_node->type == NodeType::NUMBER ||
        current_node->type == NodeType::STRING ||
        current_node->type == NodeType::BOOLEAN ||
        current_node->type == NodeType::OBJECT ||
        current_node->type == NodeType::PAREN ||
        current_node->type == NodeType::FUNC) 
        && peek()->type == NodeType::LIST) {
            node_ptr accessor = new_accessor_node();
            accessor->_Node.Accessor().container = nodes[index];
            accessor->_Node.Accessor().accessor = peek();
            nodes[index] = accessor;
            erase_next();
        }
        advance();
    }
}

void Parser::parse_var(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "var") {
            node_ptr next = peek();
            if (next->type == NodeType::ID) {
                current_node->type = NodeType::VARIABLE_DECLARATION;
                current_node->_Node = VariableDeclatationNode();
                current_node->_Node.VariableDeclaration().name = next->_Node.ID().value;
                current_node->_Node.VariableDeclaration().value = new_node(NodeType::NONE);
                erase_next();
            } else if (next->type == NodeType::OP && next->_Node.Op().value == "=") {
                current_node->type = NodeType::VARIABLE_DECLARATION;
                current_node->_Node = VariableDeclatationNode();
                if (next->_Node.Op().left->type == NodeType::OP && next->_Node.Op().left->_Node.Op().value == ":") {
                    node_ptr typed_var = next->_Node.Op().left;
                    current_node->_Node.VariableDeclaration().name = typed_var->_Node.Op().left->_Node.ID().value;
                    current_node->_Node.VariableDeclaration().value = next->_Node.Op().right;
                    current_node->TypeInfo.type = typed_var->_Node.Op().right;
                    current_node->TypeInfo.type->TypeInfo.is_type = true;
                } else {
                    current_node->_Node.VariableDeclaration().name = next->_Node.Op().left->_Node.ID().value;
                    current_node->_Node.VariableDeclaration().value = next->_Node.Op().right;
                }
                erase_next();
            } else if (next->type == NodeType::OP && next->_Node.Op().value == ":") {
                current_node->type = NodeType::VARIABLE_DECLARATION;
                current_node->_Node = VariableDeclatationNode();
                current_node->_Node.VariableDeclaration().name = next->_Node.Op().left->_Node.ID().value;
                current_node->_Node.VariableDeclaration().value = new_node(NodeType::NONE);
                current_node->TypeInfo.type = next->_Node.Op().right;
                current_node->TypeInfo.type->TypeInfo.is_type = true;
                erase_next();
            }
        }
        advance();
    }
}

void Parser::parse_const(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "const") {
            node_ptr next = peek();
            if (next->type == NodeType::OP && next->_Node.Op().value == "=") {
                current_node->type = NodeType::CONSTANT_DECLARATION;
                current_node->_Node = ConstantDeclatationNode();
                if (next->_Node.Op().left->type == NodeType::OP && next->_Node.Op().left->_Node.Op().value == ":") {
                    node_ptr typed_var = next->_Node.Op().left;
                    current_node->_Node.ConstantDeclatation().name = typed_var->_Node.Op().left->_Node.ID().value;
                    current_node->_Node.ConstantDeclatation().value = next->_Node.Op().right;
                    current_node->TypeInfo.type = typed_var->_Node.Op().right;
                    current_node->TypeInfo.type->TypeInfo.is_type = true;
                } else {
                    current_node->_Node.ConstantDeclatation().name = next->_Node.Op().left->_Node.ID().value;
                    current_node->_Node.ConstantDeclatation().value = next->_Node.Op().right;
                }
                erase_next();
            } else {
                error_and_exit("Const declaration expects a value");
            }
        }
        advance();
    }
}

void Parser::parse_import(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "import") {
            current_node->type = NodeType::IMPORT;
            current_node->_Node = ImportNode();
            if (peek()->type == NodeType::OP && peek()->_Node.Op().value == ":") {
                current_node->_Node.Import().module = peek()->_Node.Op().left;
                current_node->_Node.Import().target = peek()->_Node.Op().right;
                erase_next();
            } else if (peek()->type == NodeType::ID) {
                current_node->_Node.Import().module = peek();
                current_node->_Node.Import().is_default = true;
                erase_next();
            } else {
                error_and_exit("Malformed import statement");
            }
        }
        advance();
    }
}

void Parser::parse_for_loop(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "for") {
            current_node->type = NodeType::FOR_LOOP;
            current_node->_Node = ForLoopNode();
            if (peek()->type != NodeType::PAREN && peek(2)->type != NodeType::OBJECT) {
                error_and_exit("Malformed for loop");
            }
            
            node_ptr for_loop_config = peek();

            if (for_loop_config->_Node.Paren().elements.size() == 0 || for_loop_config->_Node.Paren().elements.size() > 3) {
                error_and_exit("For loop constructor expects 1, 2 or 3 elements");
            }

            if (for_loop_config->_Node.Paren().elements[0]->type == NodeType::COMMA_LIST) {
                for_loop_config->_Node.Paren().elements = for_loop_config->_Node.Paren().elements[0]->_Node.List().elements;
            } 
            
            if (for_loop_config->_Node.Paren().elements.size() > 0) {
                if (for_loop_config->_Node.Paren().elements[0]->type == NodeType::NUMBER) {
                    current_node->_Node.ForLoop().start = new_number_node(0);
                    current_node->_Node.ForLoop().end = for_loop_config->_Node.Paren().elements[0];
                } else if (for_loop_config->_Node.Paren().elements[0]->type == NodeType::OP && for_loop_config->_Node.Paren().elements[0]->_Node.Op().value == "..") {
                    current_node->_Node.ForLoop().start = for_loop_config->_Node.Paren().elements[0]->_Node.Op().left;
                    current_node->_Node.ForLoop().end = for_loop_config->_Node.Paren().elements[0]->_Node.Op().right;
                } else {
                    current_node->_Node.ForLoop().iterator = for_loop_config->_Node.Paren().elements[0];
                }
            }

            if (for_loop_config->_Node.Paren().elements.size() > 1) {
                if (for_loop_config->_Node.Paren().elements[1]->type != NodeType::ID) {
                    error_and_exit("Index variable in for loop must be an identifier");
                }

                current_node->_Node.ForLoop().index_name = for_loop_config->_Node.Paren().elements[1];
            }

            if (for_loop_config->_Node.Paren().elements.size() > 2) {
                if (for_loop_config->_Node.Paren().elements[2]->type != NodeType::ID) {
                    error_and_exit("Value variable in for loop must be an identifier");
                }

                current_node->_Node.ForLoop().value_name = for_loop_config->_Node.Paren().elements[2];
            }

            current_node->_Node.ForLoop().body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_while_loop(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "while") {
            current_node->type = NodeType::WHILE_LOOP;
            current_node->_Node = WhileLoopNode();
            if (peek()->type != NodeType::PAREN && peek(2)->type != NodeType::OBJECT) {
                error_and_exit("Malformed while loop");
            }
            
            node_ptr while_loop_config = peek();

            if (while_loop_config->_Node.Paren().elements.size() != 1) {
                error_and_exit("While loop constructor expects 1 element");
            }

            current_node->_Node.WhileLoop().condition = while_loop_config->_Node.Paren().elements[0];
            current_node->_Node.WhileLoop().body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_if_statement(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "if") {
            current_node->type = NodeType::IF_STATEMENT;
            current_node->_Node = IfStatementNode();
            if (peek()->type != NodeType::PAREN && peek(2)->type != NodeType::OBJECT) {
                error_and_exit("Malformed if statement");
            }
            
            node_ptr conditional = peek();

            if (conditional->_Node.Paren().elements.size() != 1) {
                error_and_exit("If statement expects 1 conditional statement");
            }

            current_node->_Node.IfStatement().condition = conditional->_Node.Paren().elements[0];
            current_node->_Node.IfStatement().body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_try_catch(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "try") {
            node_ptr try_block = peek();
            if (try_block->type != NodeType::OBJECT) {
                error_and_exit("Malformed try-catch expression - missing 'try' block");
            }
            node_ptr catch_keyword = peek(2);
            if (catch_keyword->type != NodeType::FUNC_CALL && catch_keyword->_Node.FunctionCall().name != "catch") {
                error_and_exit("Malformed try-catch expression - missing 'catch' keyword");
            }
            if (catch_keyword->_Node.FunctionCall().args.size() != 1) {
                error_and_exit("Malformed try-catch expression - 'catch' expects one argument");
            }
            if (catch_keyword->_Node.FunctionCall().args[0]->type != NodeType::ID) {
                error_and_exit("Malformed try-catch expression - 'catch' expects argument to be an identifier");
            }
            node_ptr catch_block = peek(3);
            if (catch_block->type != NodeType::OBJECT) {
                error_and_exit("Malformed try-catch expression - missing 'catch' block");
            }
            current_node->type = NodeType::TRY_CATCH;
            current_node->_Node = TryCatchNode();
            current_node->_Node.TryCatch().try_body = try_block;
            current_node->_Node.TryCatch().catch_keyword = catch_keyword;
            current_node->_Node.TryCatch().catch_body = catch_block;
            erase_next();
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_if_block(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "else") {
            current_node->type = NodeType::IF_BLOCK;
            current_node->_Node = IfBlockNode();
            node_ptr prev = peek(-1);
            node_ptr next = peek();
            if (prev->type != NodeType::IF_STATEMENT && prev->type != NodeType::IF_BLOCK) {
                error_and_exit("Malformed if block");
            } else if (next->type != NodeType::IF_STATEMENT && next->type != NodeType::OBJECT) {
                error_and_exit("Malformed if block");
            }

            if (prev->type == NodeType::IF_BLOCK) {
                for (node_ptr& statement : prev->_Node.IfBlock().statements) {
                    current_node->_Node.IfBlock().statements.push_back(statement);
                }
            } else {
                current_node->_Node.IfBlock().statements.push_back(prev);
            }

            current_node->_Node.IfBlock().statements.push_back(next);

            erase_prev();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_return(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID && current_node->_Node.ID().value == "ret") {
            current_node->type = NodeType::RETURN;
            current_node->_Node = ReturnNode();
            if (peek()->type == NodeType::OP && (peek()->_Node.Op().value == ";" || peek()->_Node.Op().value == "}")) {
                advance();
                continue;
            } else {
                current_node->_Node.Return().value = peek();
                erase_next();
            }
        }
        advance();
    }
}

void Parser::parse_keywords(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }

        if (current_node->type == NodeType::ID) {
            if (current_node->_Node.ID().value == "break") {
                current_node->type = NodeType::BREAK;
            } else if (current_node->_Node.ID().value == "continue") {
                current_node->type = NodeType::CONTINUE;
            } else if (current_node->_Node.ID().value == "Number") {
                current_node->type = NodeType::NUMBER;
                current_node->_Node = NumberNode();
                current_node->TypeInfo.is_type = true;
            } else if (current_node->_Node.ID().value == "String") {
                current_node->type = NodeType::STRING;
                current_node->_Node = StringNode();
                current_node->TypeInfo.is_type = true;
            } else if (current_node->_Node.ID().value == "Boolean") {
                current_node->type = NodeType::BOOLEAN;
                current_node->_Node = BooleanNode();
                current_node->TypeInfo.is_type = true;
            } else if (current_node->_Node.ID().value == "List") {
                current_node->type = NodeType::LIST;
                current_node->_Node = ListNode();
                current_node->TypeInfo.is_type = true;
                current_node->TypeInfo.is_general_type = true;
            } else if (current_node->_Node.ID().value == "Object") {
                current_node->type = NodeType::OBJECT;
                current_node->_Node = ObjectNode();
                current_node->TypeInfo.is_type = true;
            } else if (current_node->_Node.ID().value == "Function") {
                current_node->type = NodeType::FUNC;
                current_node->_Node = FuncNode();
                current_node->TypeInfo.is_type = true;
                current_node->TypeInfo.is_general_type = true;
            } else if (current_node->_Node.ID().value == "Pointer") {
                current_node->type = NodeType::POINTER;
                current_node->_Node = PointerNode();
                current_node->TypeInfo.is_type = true;
            } else if (current_node->_Node.ID().value == "Library") {
                current_node->type = NodeType::LIB;
                current_node->_Node = LibNode();
                current_node->TypeInfo.is_type = true;
            } else if (current_node->_Node.ID().value == "Any") {
                current_node->type = NodeType::ANY;
                current_node->TypeInfo.is_type = true;
            } else if (current_node->_Node.ID().value == "Error") {
                current_node->type = NodeType::ERROR;
                current_node->_Node = ErrorNode();
                current_node->TypeInfo.is_type = true;
            }
        }
        advance();
    }
}

void Parser::flatten_commas(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == end) {
            break;
        }
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == ",") {
            current_node = flatten_comma_node(current_node);
        }
        advance();
    }
}

void Parser::parse(int start, std::string end) {
    parse_paren(end);
    reset(start);
    parse_object(end);
    reset(start);
    parse_enum(end);
    reset(start);
    parse_union(end);
    reset(start);
    parse_list(end);
    reset(start);
    parse_keywords(end);
    reset(start);
    parse_for_loop(end);
    reset(start);
    parse_while_loop(end);
    reset(start);
    parse_if_statement(end);
    reset(start);
    parse_if_block(end);
    reset(start);
    parse_func_call(end);
    reset(start);
    parse_post_op({"?"}, end);
    reset(start);
    // parse_type_ext(end);
    // reset(start);
    parse_try_catch(end);
    reset(start);
    parse_accessor(end);
    reset(start);
    parse_dot(end);
    reset(start);
    parse_un_op({"!"}, end);
    reset(start);
    parse_un_op_amb({"+", "-"}, end);
    reset(start);
    parse_un_op_amb({"&"}, end);
    reset(start);
    parse_bin_op({"??"}, end);
    reset(start);
    parse_bin_op({"^"}, end);
    reset(start);
    parse_bin_op({"*", "/", "%"}, end);
    reset(start);
    parse_bin_op({"+", "-"}, end);
    reset(start);
    parse_bin_op({"+=", "-="}, end);
    reset(start);
    parse_bin_op({"==", "!=", "<=", ">=", "<", ">"}, end);
    reset(start);
    parse_bin_op({"&&", "||", "??"}, end);
    reset(start);
    parse_bin_op({"&", "|"}, end);
    reset(start);
    parse_bin_op({".."}, end);
    reset(start);
    parse_object_desconstruct(end);
    reset(start);
    parse_func_def(end);
    reset(start);
    parse_type(end);
    reset(start);
    parse_hook_implementation(end);
    reset(start);
    parse_bin_op({"::"}, end);
    reset(start);
    parse_colon(end);
    reset(start);
    parse_equals(end);
    reset(start);
    parse_comma(end);
    reset(start);
    flatten_commas(end);
    reset(start);
    parse_var(end);
    reset(start);
    parse_const(end);
    reset(start);
    parse_return(end);
    reset(start);
    parse_import(end);
    reset(start);
}

int Parser::find_closing_index(int start, std::string opening_symbol, std::string closing_symbol) {
    int count = 0;

    for (int i = start; i < nodes.size(); i++) {
        if (nodes[i]->type == NodeType::OP && nodes[i]->_Node.Op().value == opening_symbol) {
            count++;
        } else if (nodes[i]->type == NodeType::OP && nodes[i]->_Node.Op().value == closing_symbol) {
            count--;
        }

        if (nodes[i]->type == NodeType::OP && nodes[i]->_Node.Op().value == closing_symbol && count == 0) {
            return i;
        }
    }

    return 0;
}

node_ptr Parser::flatten_comma_node(node_ptr node) {
    // node->type = NodeType::COMMA_LIST;
    node_ptr comma_list = new_node(NodeType::LIST);
    comma_list->type = NodeType::COMMA_LIST;

    if (node->type == NodeType::OP && node->_Node.Op().left->type == NodeType::OP && node->_Node.Op().left->_Node.Op().value == ",") {
        node->_Node.Op().left = flatten_comma_node(node->_Node.Op().left);
    } else {
        comma_list->_Node.List().elements.push_back(node->_Node.Op().left);
    }

    if (node->type == NodeType::OP && node->_Node.Op().left->type == NodeType::COMMA_LIST) {
        for (auto& child_node : node->_Node.Op().left->_Node.List().elements) {
            comma_list->_Node.List().elements.push_back(child_node);
        }
    }


    if (node->type == NodeType::OP && node->_Node.Op().right->type == NodeType::OP && node->_Node.Op().right->_Node.Op().value == ",") {
        node->_Node.Op().right = flatten_comma_node(node->_Node.Op().right);
    } else {
        comma_list->_Node.List().elements.push_back(node->_Node.Op().right);
    }

    if (node->type == NodeType::OP && node->_Node.Op().right->type == NodeType::COMMA_LIST) {
        for (auto& child_node : node->_Node.Op().right->_Node.List().elements) {
            comma_list->_Node.List().elements.push_back(child_node);
        }
    }

    *node = *comma_list;
    
    return node;
}

void Parser::remove_op_node(std::string type) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->type == NodeType::OP && current_node->_Node.Op().value == type) {
            erase_curr();
        }

        advance();
    }
}

void Parser::erase_next() {
    nodes.erase(nodes.begin() + index + 1);
}

void Parser::erase_prev() {
    nodes.erase(nodes.begin() + index - 1);
    index--;
    current_node = nodes[index];
}

void Parser::erase_curr() {
    nodes.erase(nodes.begin() + index);
    index--;
    current_node = nodes[index];
}

bool Parser::has_children(node_ptr node) {
    return (node->type == NodeType::OP && node->_Node.Op().left || node->_Node.Op().right);
}

void Parser::error_and_exit(std::string message)
{
    std::string error_message = "Parsing Error in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message << "\n";
    exit(1);
}

node_ptr Parser::new_number_node(double value) {
    auto node = std::make_shared<Node>(NodeType::NUMBER);
    node->_Node.Number().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Parser::new_string_node(std::string value) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->_Node.String().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Parser::new_boolean_node(bool value) {
    auto node = std::make_shared<Node>(NodeType::BOOLEAN);
    node->_Node.Boolean().value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Parser::new_accessor_node() {
    auto node = std::make_shared<Node>(NodeType::ACCESSOR);
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Parser::new_node() {
    auto node = std::make_shared<Node>();
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Parser::new_node(NodeType type) {
    auto node = std::make_shared<Node>(type);
    node->line = line;
    node->column = column;
    return node;
}