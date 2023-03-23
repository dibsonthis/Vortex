#include <vector>
#include <iostream>

#define node_ptr std::shared_ptr<Node>

enum class NodeType {
	ID,
    NUMBER,
    STRING,
	BOOLEAN,
	OP,
	START_OF_FILE,
	END_OF_FILE,
};

struct IdNode {
	std::string value;
};

struct NumberNode {
    double value;
};

struct StringNode {
    std::string value;
};

struct BooleanNode {
    bool value;
};

struct OpNode {
	std::string value;
};

struct Node {
	Node() = default;
    Node(NodeType type) : type(type) {}
	Node(int line, int column) : line(line), column(column) {}
	Node(NodeType type, int line, int column) : type(type), line(line), column(column) {}

    NodeType type;
	int column = 1;
	int line = 1;

	IdNode ID;
    NumberNode Number;
    StringNode String;
	BooleanNode Boolean;
	OpNode Operator;

	bool __parsed;
};

node_ptr new_number_node(double value);
node_ptr new_string_node(std::string value);
node_ptr new_boolean_node(bool value);