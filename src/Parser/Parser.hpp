#pragma once

#include "../Node/Node.hpp"
#include "../utils/utils.hpp"

class Parser {
public:
    std::vector<node_ptr> nodes;
    node_ptr current_node;
    int index = 0;
    std::string file_name;
    int line, column;

public:
    Parser() = default;
    Parser(std::vector<node_ptr> nodes) : nodes(nodes) {
        current_node = nodes[0];
    }
    void advance(int n = 1);
    node_ptr peek(int n = 1);
    void reset(int idx = 0);

    void parse_list(int end);
    void parse_paren(int end);
    void parse_bin_op(std::vector<std::string> operators, int end);
    void parse_un_op(std::vector<std::string> operators, int end);
    void parse_comma(int end);
    void parse_func_call(int end);
    void parse_accessor(int end);
    void flatten_commas(int end);
    void parse(int start, int end);

    bool has_children(node_ptr node);

    node_ptr flatten_comma_node(node_ptr node);
    int find_closing_index(int start, std::string opening_symbol, std::string closing_symbol);
    void erase_prev();
    void erase_next();
    void error_and_exit(std::string message);

    node_ptr new_number_node(double value);
    node_ptr new_string_node(std::string value);
    node_ptr new_boolean_node(bool value);
    node_ptr new_accessor_node();
};