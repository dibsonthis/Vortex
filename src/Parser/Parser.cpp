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

node_ptr Parser::get_left() {
    int idx = index;
    node_ptr node = nodes[idx - 1];
    while (node->__parsed) {
        idx--;
        node = nodes[idx];
    }
    return node;
}

node_ptr Parser::get_right() {
    int idx = index;
    node_ptr node = nodes[idx + 1];
    while (node->__parsed) {
        idx++;
        node = nodes[idx];
    }
    return node;
}

void Parser::parse_bin_op(std::vector<std::string> operators, int end) {
    while (current_node->type != NodeType::END_OF_FILE && index != end) {
        if (!current_node->__parsed && !has_children(current_node) && vector_contains_string(operators, current_node->Operator.value)) {
            node_ptr left = get_left();
            node_ptr right = get_right();
            left->__parsed = true;
            right->__parsed = true;
            current_node->Operator.left = left;
            current_node->Operator.right = right;
        }
        advance();
    }
}

void Parser::parse_un_op(std::vector<std::string> operators, int end) {
    while (current_node->type != NodeType::END_OF_FILE && index != end) {
        if (!current_node->__parsed && vector_contains_string(operators, current_node->Operator.value) && 
            (
                peek(-1)->type == NodeType::OP ||
                peek(-1)->type == NodeType::START_OF_FILE
            ) && 
            (
                peek(-1)->Operator.value != ")" &&
                peek(-1)->Operator.value != "]"
            )) {
            node_ptr right = get_right();
            right->__parsed = true;
            current_node->Operator.right = right;
        }
        advance();
    }
}

void Parser::parse_list(int end) {
    while (current_node->type != NodeType::END_OF_FILE && index != end) {
        if (current_node->Operator.value == "[") {
            current_node->type = NodeType::LIST;
            int curr_idx = index;
            int closing_index = find_closing_index(index, "[", "]");
            advance();
            parse(index, closing_index);
            for (int i = index; i < closing_index; i++) {
                if (!nodes[i]->__parsed) {
                    nodes[curr_idx]->List.elements.push_back(nodes[i]);
                    nodes[i]->__parsed = true;
                }
            }
            reset(closing_index);
            current_node->__parsed = true; // "]"
        }

        advance();
    }
}

void Parser::parse_paren(int end) {
    while (current_node->type != NodeType::END_OF_FILE && index != end) {
        if (current_node->Operator.value == "(") {
            current_node->type = NodeType::PAREN;
            int curr_idx = index;
            int closing_index = find_closing_index(index, "(", ")");
            advance();
            parse(index, closing_index);
            for (int i = index; i < closing_index; i++) {
                if (!nodes[i]->__parsed) {
                    nodes[curr_idx]->Paren.elements.push_back(nodes[i]);
                    nodes[i]->__parsed = true;
                }
            }
            reset(closing_index);
            current_node->__parsed = true; // ")"
        }

        advance();
    }
}

void Parser::parse_func_call(int end) {
    while (current_node->type != NodeType::END_OF_FILE && index != end) {
        if (!current_node->__parsed && current_node->type == NodeType::ID && peek()->type == NodeType::PAREN) {
            current_node->type = NodeType::FUNC_CALL;
            current_node->FuncCall.name = current_node->ID.value;
            current_node->FuncCall.args.push_back(peek());
            advance();
            current_node->__parsed = true;
        }
        advance();
    }
}

void Parser::flatten_commas(int end) {
    while (current_node->type != NodeType::END_OF_FILE && index != end) {
        if (!current_node->__parsed && current_node->Operator.value == ",") {
            current_node = flatten_comma_node(current_node);
        }
        advance();
    }
}

void Parser::parse(int start, int end) {
    parse_paren(end);
    reset(start);
    parse_list(end);
    reset(start);
    parse_bin_op({","}, end);
    reset(start);
    parse_func_call(end);
    reset(start);
    parse_un_op({"+", "-"}, end);
    reset(start);
    parse_bin_op({"&&", "||"}, end);
    reset(start);
    parse_bin_op({"*", "/"}, end);
    reset(start);
    parse_bin_op({"+", "-"}, end);
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

std::vector<node_ptr> Parser::filter_tree() {
    std::vector<node_ptr> ast;
    for (node_ptr& node : nodes) {
        if (!node->__parsed) {
            ast.push_back(node);
        }
    }
    return ast;
}

bool Parser::has_children(node_ptr node) {
    return (node->Operator.left || node->Operator.right);
}

void Parser::remove_parsed() {
    nodes.erase(
    std::remove_if(
        nodes.begin(), 
        nodes.end(),
        [](node_ptr const & node) { return node->__parsed; }
    ), 
    nodes.end()
); 
}

void Parser::error_and_exit(std::string message)
{
    std::string error_message = "Parsing Error in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message << "\n";
    exit(1);
}