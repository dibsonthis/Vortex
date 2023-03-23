#pragma once

#include "../Node/Node.hpp"
#include "../utils/utils.hpp"

class Parser {
    std::vector<node_ptr> nodes;
    node_ptr current_node;
    int index = 0;

public:
    Parser() = default;
    Parser(std::vector<node_ptr> nodes) : nodes(nodes) {
        current_node = nodes[0];
    }
    void advance(int n = 1);
    void run();

    node_ptr get_left();
    node_ptr get_right();

    void reset(int idx = 0);

    void parse_bin_op(std::vector<std::string> operators);

    std::vector<node_ptr> filter_tree();
};