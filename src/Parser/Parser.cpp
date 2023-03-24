#include "Parser.hpp"

void Parser::advance(int n) {
    index += n;
    if (index < nodes.size()) {
        current_node = nodes[index];
        line = current_node->line;
        column = current_node->column;
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
        if (!current_node->__parsed && vector_contains_string(operators, current_node->Operator.value)) {
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

void Parser::parse(int start, int end) {
    parse_list(end);
    reset(start);
    parse_bin_op({"*", "/"}, end);
    reset(start);
    parse_bin_op({"+", "-"}, end);
    reset(start);
    parse_bin_op({"="}, end);
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

std::vector<node_ptr> Parser::filter_tree() {
    std::vector<node_ptr> ast;
    for (node_ptr& node : nodes) {
        if (!node->__parsed) {
            ast.push_back(node);
        }
    }
    return ast;
}

void Parser::error_and_exit(std::string message)
{
    std::string error_message = "Parsing Error in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message << "\n";
    exit(1);
}