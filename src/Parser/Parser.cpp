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
                peek(-1)->type == NodeType::OP ||
                peek(-1)->type == NodeType::START_OF_FILE
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
                nodes[curr_idx]->Object.elements.push_back(peek());
                erase_next();
            }
            erase_next();
        }

        if (current_node->Object.elements.size() == 1 && current_node->Object.elements[0]->type == NodeType::COMMA_LIST) {
            current_node->Object.elements = current_node->Object.elements[0]->List.elements;
        }

        advance();
    }
}

void Parser::parse_interface(std::string end) {
    while (current_node->type != NodeType::END_OF_FILE) {
        if (current_node->Operator.value == end) {
            break;
        }
        if (current_node->type == NodeType::ID && current_node->ID.value == "interface" && peek()->type == NodeType::ID && peek(2)->type == NodeType::OBJECT) {
            current_node->type = NodeType::INTERFACE;
            current_node->Interface.name = peek()->ID.value;
            current_node->Interface.elements = peek(2)->Object.elements;
            erase_next();
            erase_next();
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
            current_node->FuncCall.args.push_back(peek());
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
        while ((current_node->type == NodeType::ID || current_node->type == NodeType::LIST || current_node->type == NodeType::FUNC_CALL || current_node->type == NodeType::ACCESSOR) 
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
    parse_interface(end);
    reset(start);
    parse_list(end);
    reset(start);
    parse_func_call(end);
    reset(start);
    parse_accessor(end);
    reset(start);
    parse_post_op({"?"}, end);
    reset(start);
    parse_un_op({"+", "-"}, end);
    reset(start);
    parse_bin_op({"&&", "||"}, end);
    reset(start);
    parse_bin_op({"*", "/"}, end);
    reset(start);
    parse_bin_op({"+", "-"}, end);
    reset(start);
    parse_bin_op({"."}, end);
    reset(start);
    parse_bin_op({":"}, end);
    reset(start);
    parse_comma(end);
    reset(start);
    parse_bin_op({"="}, end);
    reset(start);
    flatten_commas(end);
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