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
}

void Parser::reset(int idx) {
    index = idx;
    current_node = nodes[index];
}

void Parser::parse_comma(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (!has_children(current_node) && current_node->Operator.value == ",") {
            node_ptr left = peek(-1);
            node_ptr right = peek(1);

            if ((left->type == NodeType::OP && !has_children(left)) || (right->type == NodeType::OP && !has_children(right))) {
                erase_curr();
                continue;
            }

            current_node->Operator.left = left;
            current_node->Operator.right = right;
            erase_next();
            erase_prev();
        }
        advance();
    }
}

void Parser::parse_bin_op(std::vector<std::string> operators, std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (!has_children(current_node) && vector_contains_string(operators, current_node->Operator.value)) {
            node_ptr left = peek(-1);
            node_ptr right = peek(1);
            current_node->Operator.left = left;
            current_node->Operator.right = right;
            erase_next();
            erase_prev();
        }
        advance();
    }
}

void Parser::parse_un_op(std::vector<std::string> operators, std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (vector_contains_string(operators, current_node->Operator.value) && 
            (
                (peek(-1)->type == NodeType::OP && !has_children(peek(-1))) ||
                peek(-1)->type == NodeType::START_OF_FILE ||
                peek(-1)->type == NodeType::PAREN ||
                peek(-1)->type == NodeType::LIST
            )) {
            node_ptr right = peek(1);
            current_node->Operator.right = right;
            erase_next();
        }
        advance();
    }
}

void Parser::parse_post_op(std::vector<std::string> operators, std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (vector_contains_string(operators, current_node->Operator.value) && 
            (
                peek(1)->type == NodeType::OP ||
                peek(1)->type == NodeType::END_OF_FILE
            )) {
            node_ptr left = peek(-1);
            current_node->Operator.right = left;
            erase_prev();
        }
        advance();
    }
}

void Parser::parse_equals(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (!has_children(current_node) && current_node->Operator.value == "=") {
            node_ptr left = peek(-1);
            node_ptr right = peek(1);
            current_node->Operator.left = left;
            current_node->Operator.right = right;
            erase_next();
            erase_prev();

            if (current_node->Operator.right->type == NodeType::FUNC) {
                if (left->type == NodeType::ID) {
                    current_node->Operator.right->Function.name = left->ID.value;
                } else if (left->Operator.value == ".") {
                    current_node->Operator.right->Function.name = left->Operator.right->ID.value;
                }
            }
        }
        advance();
    }
}

void Parser::parse_colon(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (!has_children(current_node) && current_node->Operator.value == ":") {
            node_ptr left = peek(-1);
            node_ptr right = peek(1);
            current_node->Operator.left = left;
            current_node->Operator.right = right;
            erase_next();
            erase_prev();

            if (current_node->Operator.right->type == NodeType::FUNC) {
                current_node->Operator.right->Function.name = left->ID.value;
            }
        }
        advance();
    }
}

void Parser::parse_list(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->Operator.value == "[") {
            current_node->type = NodeType::LIST;
            int curr_idx = index;
            int closing_index = find_closing_index(index, "[", "]");
            current_node->Operator.value = "";
            advance();
            parse(index, "]");
            advance(-1);
            while (peek()->Operator.value != "]") {
                if (peek()->type == NodeType::END_OF_FILE) {
                    error_and_exit("Missing end ']'");
                }
                nodes[curr_idx]->List.elements.push_back(peek());
                erase_next();
            }
            erase_next();
        }

        advance();
    }
}

void Parser::parse_object(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->Operator.value == "{") {
            current_node->type = NodeType::OBJECT;
            int curr_idx = index;
            int closing_index = find_closing_index(index, "{", "}");
            current_node->Operator.value = "";
            advance();
            parse(index, "}");
            advance(-1);
            while (peek()->Operator.value != "}") {
                if (peek()->type == NodeType::END_OF_FILE) {
                    error_and_exit("Missing end '}'");
                }
                nodes[curr_idx]->Object.elements.push_back(peek());
                erase_next();
            }
            erase_next();
        }

        advance();
    }
}

void Parser::parse_enum(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && current_node->ID.value == "enum" && peek()->type == NodeType::ID && peek(2)->type == NodeType::OBJECT) {
            current_node->type = NodeType::ENUM;
            current_node->Enum.name = peek()->ID.value;
            current_node->Enum.body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_type(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && current_node->ID.value == "type" && (peek()->type == NodeType::ID || peek()->type == NodeType::ACCESSOR) && peek(2)->type == NodeType::OBJECT) {
            current_node->type = NodeType::TYPE;
            current_node->Type.name = peek()->ID.value;
            current_node->Type.body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_hook_implementation(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->Operator.value == "::" && peek(-1)->type == NodeType::ID && peek()->type == NodeType::FUNC) {
            current_node->type = NodeType::HOOK;
            current_node->Operator.value = "";
            current_node->Hook.hook_name = peek(-1)->ID.value;
            current_node->Hook.function = peek(1);
            erase_next();
            erase_prev();
        }
        advance();
    }
}

void Parser::parse_func_def(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->Operator.value == "=>" && peek()->type != NodeType::END_OF_FILE) {
            if (peek(-1)->type == NodeType::PAREN)
            {
                current_node->type = NodeType::FUNC;
                node_ptr params = peek(-1);
                if (params->Paren.elements.size() == 1 && params->Paren.elements[0]->type == NodeType::COMMA_LIST) {
                    for (node_ptr& elem : params->Paren.elements[0]->List.elements) {
                        current_node->Function.params.push_back(elem);
                        current_node->Function.args.push_back(nullptr);
                    }
                } else if (params->Paren.elements.size() == 1) {
                    current_node->Function.params.push_back(params->Paren.elements[0]);
                    current_node->Function.args.push_back(nullptr);
                }
                current_node->Function.body = peek();
                erase_next();
                erase_prev();
            } else if ( 
                peek(-2)->Operator.value == ":" && 
                peek(-3)->type == NodeType::PAREN)
            {
                current_node->type = NodeType::FUNC;
                node_ptr return_type = peek(-1);
                node_ptr params = peek(-3);

                if (params->Paren.elements.size() == 1 && params->Paren.elements[0]->type == NodeType::COMMA_LIST) {
                    for (node_ptr& elem : params->Paren.elements[0]->List.elements) {
                        current_node->Function.params.push_back(elem);
                        current_node->Function.args.push_back(nullptr);
                    }
                } else if (params->Paren.elements.size() == 1) {
                    current_node->Function.params.push_back(params->Paren.elements[0]);
                    current_node->Function.args.push_back(nullptr);
                }
                current_node->Function.body = peek();
                current_node->Function.return_type = return_type;
                erase_next();
                erase_prev();
                erase_prev();
                erase_prev();
            }
        }
        advance();
    }
}

void Parser::parse_paren(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->Operator.value == "(") {
            current_node->type = NodeType::PAREN;
            int curr_idx = index;
            int closing_index = find_closing_index(index, "(", ")");
            current_node->Operator.value = "";
            advance();
            parse(index, ")");
            advance(-1);
            while (peek()->Operator.value != ")") {
                if (peek()->type == NodeType::END_OF_FILE) {
                    error_and_exit("Missing end ')'");
                }
                nodes[curr_idx]->Paren.elements.push_back(peek());
                erase_next();
            }
            erase_next();
        }

        advance();
    }
}

void Parser::parse_func_call(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && peek()->type == NodeType::PAREN) {
            current_node->type = NodeType::FUNC_CALL;
            current_node->FuncCall.name = current_node->ID.value;
            node_ptr argsList = peek();
            if (argsList->Paren.elements.size() == 1 && argsList->Paren.elements[0]->type == NodeType::COMMA_LIST) {
                current_node->FuncCall.args = argsList->Paren.elements[0]->List.elements;
            } else if (argsList->Paren.elements.size() == 1) {
                current_node->FuncCall.args.push_back(argsList->Paren.elements[0]);
            }
            erase_next();
        }
        advance();
    }
}

void Parser::parse_object_desconstruct(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && peek()->type == NodeType::OBJECT) {
            current_node->type = NodeType::OBJECT_DECONSTRUCT;
            current_node->ObjectDeconstruct.name = current_node->ID.value;
            current_node->ObjectDeconstruct.body = peek();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_accessor(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->ID.value == "import") {
            advance();
            continue;
        }
        while ((current_node->type == NodeType::ID || current_node->type == NodeType::LIST || current_node->type == NodeType::FUNC_CALL || current_node->type == NodeType::ACCESSOR || current_node->Operator.value == "." || 
        current_node->type == NodeType::NUMBER ||
        current_node->type == NodeType::STRING ||
        current_node->type == NodeType::BOOLEAN ||
        current_node->type == NodeType::OBJECT ||
        current_node->type == NodeType::FUNC) 
        && peek()->type == NodeType::LIST) {
            node_ptr accessor = new_accessor_node();
            accessor->Accessor.container = nodes[index];
            accessor->Accessor.accessor = peek();
            nodes[index] = accessor;
            erase_next();
        }
        advance();
    }
}

void Parser::parse_var(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }

        if (current_node->ID.value == "var") {
            node_ptr next = peek();
            if (next->type == NodeType::COMMA_LIST) {
                current_node->type = NodeType::VARIABLE_DECLARATION_MULTIPLE;
                for (node_ptr& element : next->List.elements) {
                    node_ptr var_decl = new_node();
                    var_decl->type = NodeType::VARIABLE_DECLARATION;
                    var_decl->VariableDeclaration.name = element->ID.value;
                    var_decl->VariableDeclaration.value = new_node(NodeType::NONE);
                    current_node->VariableDeclarationMultiple.variable_declarations.push_back(var_decl);
                }
                erase_next();
            } else if (next->Operator.value == "=" && next->Operator.left->type == NodeType::COMMA_LIST) {
                current_node->type = NodeType::VARIABLE_DECLARATION_MULTIPLE;
                for (node_ptr& element : next->Operator.left->List.elements) {
                    node_ptr var_decl = new_node();
                    var_decl->type = NodeType::VARIABLE_DECLARATION;
                    var_decl->VariableDeclaration.name = element->ID.value;
                    var_decl->VariableDeclaration.value = std::make_shared<Node>(*next->Operator.right);
                    current_node->VariableDeclarationMultiple.variable_declarations.push_back(var_decl);
                }
                erase_next();
            } else if (next->type == NodeType::ID) {
                current_node->type = NodeType::VARIABLE_DECLARATION;
                current_node->VariableDeclaration.name = next->ID.value;
                current_node->VariableDeclaration.value = new_node(NodeType::NONE);
                erase_next();
            } else if (next->Operator.value == "=") {
                current_node->type = NodeType::VARIABLE_DECLARATION;
                current_node->VariableDeclaration.name = next->Operator.left->ID.value;
                current_node->VariableDeclaration.value = next->Operator.right;
                erase_next();
            }
        }
        advance();
    }
}

void Parser::parse_const(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }

        if (current_node->ID.value == "const") {
            node_ptr next = peek();
            if (next->Operator.value == "=" && next->Operator.left->type == NodeType::COMMA_LIST) {
                current_node->type = NodeType::CONSTANT_DECLARATION_MULTIPLE;
                for (node_ptr& element : next->Operator.left->List.elements) {
                    node_ptr const_decl = new_node();
                    const_decl->type = NodeType::CONSTANT_DECLARATION;
                    const_decl->ConstantDeclaration.name = element->ID.value;
                    const_decl->ConstantDeclaration.value = std::make_shared<Node>(*next->Operator.right);
                    current_node->ConstantDeclarationMultiple.constant_declarations.push_back(const_decl);
                }
                erase_next();
            } else if (next->Operator.value == "=") {
                current_node->type = NodeType::CONSTANT_DECLARATION;
                current_node->ConstantDeclaration.name = next->Operator.left->ID.value;
                current_node->ConstantDeclaration.value = next->Operator.right;
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
        if (current_node->Operator.value == end) {
            break;
        }

        if (current_node->ID.value == "import") {
            current_node->type = NodeType::IMPORT;
            if (peek()->Operator.value == ":") {
                current_node->Import.module = peek()->Operator.left;
                current_node->Import.target = peek()->Operator.right;
                erase_next();
            } else {
                error_and_exit("Const declaration expects a value");
            }
        }
        advance();
    }
}

void Parser::parse_for_loop(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }

        if (current_node->ID.value == "for") {
            current_node->type = NodeType::FOR_LOOP;
            if (peek()->type != NodeType::PAREN && peek(2)->type != NodeType::OBJECT) {
                error_and_exit("Malformed for loop");
            }
            
            node_ptr for_loop_config = peek();

            if (for_loop_config->Paren.elements.size() == 0 || for_loop_config->Paren.elements.size() > 3) {
                error_and_exit("For loop constructor expects 1, 2 or 3 elements");
            }

            if (for_loop_config->Paren.elements[0]->type == NodeType::COMMA_LIST) {
                for_loop_config->Paren.elements = for_loop_config->Paren.elements[0]->List.elements;
            } 
            
            if (for_loop_config->Paren.elements.size() > 0) {
                if (for_loop_config->Paren.elements[0]->type == NodeType::NUMBER) {
                    current_node->ForLoop.start = new_number_node(0);
                    current_node->ForLoop.end = for_loop_config->Paren.elements[0];
                } else if (for_loop_config->Paren.elements[0]->Operator.value == "..") {
                    current_node->ForLoop.start = for_loop_config->Paren.elements[0]->Operator.left;
                    current_node->ForLoop.end = for_loop_config->Paren.elements[0]->Operator.right;
                } else {
                    current_node->ForLoop.iterator = for_loop_config->Paren.elements[0];
                }
            }

            if (for_loop_config->Paren.elements.size() > 1) {
                if (for_loop_config->Paren.elements[1]->type != NodeType::ID) {
                    error_and_exit("Index variable in for loop must be an identifier");
                }

                current_node->ForLoop.index_name = for_loop_config->Paren.elements[1];
            }

            if (for_loop_config->Paren.elements.size() > 2) {
                if (for_loop_config->Paren.elements[2]->type != NodeType::ID) {
                    error_and_exit("Value variable in for loop must be an identifier");
                }

                current_node->ForLoop.value_name = for_loop_config->Paren.elements[2];
            }

            current_node->ForLoop.body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_while_loop(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }

        if (current_node->ID.value == "while") {
            current_node->type = NodeType::WHILE_LOOP;
            if (peek()->type != NodeType::PAREN && peek(2)->type != NodeType::OBJECT) {
                error_and_exit("Malformed while loop");
            }
            
            node_ptr while_loop_config = peek();

            if (while_loop_config->Paren.elements.size() != 1) {
                error_and_exit("While loop constructor expects 1 element");
            }

            current_node->WhileLoop.condition = while_loop_config->Paren.elements[0];
            current_node->WhileLoop.body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_if_statement(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }

        if (current_node->ID.value == "if") {
            current_node->type = NodeType::IF_STATEMENT;
            if (peek()->type != NodeType::PAREN && peek(2)->type != NodeType::OBJECT) {
                error_and_exit("Malformed if statement");
            }
            
            node_ptr conditional = peek();

            if (conditional->Paren.elements.size() != 1) {
                error_and_exit("If statement expects 1 conditional statement");
            }

            current_node->IfStatement.condition = conditional->Paren.elements[0];
            current_node->IfStatement.body = peek(2);
            erase_next();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_if_block(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }

        if (current_node->ID.value == "else") {
            current_node->type = NodeType::IF_BLOCK;
            node_ptr prev = peek(-1);
            node_ptr next = peek();
            if (prev->type != NodeType::IF_STATEMENT && prev->type != NodeType::IF_BLOCK) {
                error_and_exit("Malformed if block");
            } else if (next->type != NodeType::IF_STATEMENT && next->type != NodeType::OBJECT) {
                error_and_exit("Malformed if block");
            }

            if (prev->type == NodeType::IF_BLOCK) {
                for (node_ptr& statement : prev->IfBlock.statements) {
                    current_node->IfBlock.statements.push_back(statement);
                }
            } else {
                current_node->IfBlock.statements.push_back(prev);
            }

            current_node->IfBlock.statements.push_back(next);

            erase_prev();
            erase_next();
        }
        advance();
    }
}

void Parser::parse_return(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }

        if (current_node->ID.value == "ret") {
            current_node->type = NodeType::RETURN;
            if (peek()->Operator.value == ";" || peek()->Operator.value == "}") {
                advance();
                continue;
            } else {
                current_node->Return.value = peek();
                erase_next();
            }
        }
        advance();
    }
}

void Parser::parse_keywords(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }

        if (current_node->ID.value == "break") {
            current_node->type = NodeType::BREAK;
        } else if (current_node->ID.value == "continue") {
            current_node->type = NodeType::CONTINUE;
        } else if (current_node->ID.value == "Number") {
            current_node->type = NodeType::NUMBER;
            current_node->Number.is_type = true;
        } else if (current_node->ID.value == "String") {
            current_node->type = NodeType::STRING;
            current_node->String.is_type = true;
        } else if (current_node->ID.value == "Boolean") {
            current_node->type = NodeType::BOOLEAN;
            current_node->Boolean.is_type = true;
        } else if (current_node->ID.value == "List") {
            current_node->type = NodeType::LIST;
            current_node->List.is_type = true;
        } else if (current_node->ID.value == "Object") {
            current_node->type = NodeType::OBJECT;
            current_node->Object.is_type = true;
        } else if (current_node->ID.value == "Function") {
            current_node->type = NodeType::FUNC;
            current_node->Function.is_type = true;
        } else if (current_node->ID.value == "Pointer") {
            current_node->type = NodeType::POINTER;
            current_node->Pointer.is_type = true;
        } else if (current_node->ID.value == "Pointer") {
            current_node->type = NodeType::LIB;
            current_node->Library.is_type = true;
        } 
        advance();
    }
}

void Parser::flatten_commas(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->Operator.value == ",") {
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
    parse_bin_op({"."}, end);
    reset(start);
    parse_accessor(end);
    reset(start);
    parse_type(end);
    reset(start);
    parse_un_op({"+", "-"}, end);
    reset(start);
    parse_bin_op({"==", "!=", "<=", ">=", "<", ">"}, end);
    reset(start);
    parse_bin_op({"&&", "||"}, end);
    reset(start);
    parse_bin_op({"*", "/"}, end);
    reset(start);
    parse_bin_op({"+", "-"}, end);
    reset(start);
    parse_bin_op({"^"}, end);
    reset(start);
    parse_bin_op({"+=", "-="}, end);
    reset(start);
    parse_bin_op({".."}, end);
    reset(start);
    parse_object_desconstruct(end);
    reset(start);
    parse_func_def(end);
    reset(start);
    parse_hook_implementation(end);
    reset(start);
    parse_bin_op({"::"}, end);
    reset(start);
    parse_bin_op({"&"}, end);
    reset(start);
    parse_un_op({"!"}, end);
    reset(start);
    parse_colon(end);
    reset(start);
    parse_comma(end);
    reset(start);
    flatten_commas(end);
    reset(start);
    parse_equals(end);
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
        if (nodes[i]->Operator.value == opening_symbol) {
            count++;
        } else if (nodes[i]->Operator.value == closing_symbol) {
            count--;
        }

        if (nodes[i]->Operator.value == closing_symbol && count == 0) {
            return i;
        }
    }
}

node_ptr Parser::flatten_comma_node(node_ptr node) {
    node->type = NodeType::COMMA_LIST;
    if (node->Operator.left->Operator.value == ",") {
        node->Operator.left = flatten_comma_node(node->Operator.left);
    } else {
        node->List.elements.push_back(node->Operator.left);
    }

    if (node->Operator.left->type == NodeType::COMMA_LIST) {
        for (auto& child_node : node->Operator.left->List.elements) {
            node->List.elements.push_back(child_node);
        }
    }


    if (node->Operator.right->Operator.value == ",") {
        node->Operator.right = flatten_comma_node(node->Operator.right);
    } else {
        node->List.elements.push_back(node->Operator.right);
    }

    if (node->Operator.right->type == NodeType::COMMA_LIST) {
        for (auto& child_node : node->Operator.right->List.elements) {
            node->List.elements.push_back(child_node);
        }
    }
    
    return node;
}

void Parser::remove_op_node(std::string type) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == type) {
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
    return (node->Operator.left || node->Operator.right);
}

void Parser::error_and_exit(std::string message)
{
    std::string error_message = "Parsing Error in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message << "\n";
    exit(1);
}

node_ptr Parser::new_number_node(double value) {
    auto node = std::make_shared<Node>(NodeType::NUMBER);
    node->Number.value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Parser::new_string_node(std::string value) {
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->String.value = value;
    node->line = line;
    node->column = column;
    return node;
}

node_ptr Parser::new_boolean_node(bool value) {
    auto node = std::make_shared<Node>(NodeType::BOOLEAN);
    node->Boolean.value = value;
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