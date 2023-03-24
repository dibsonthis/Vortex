#pragma once

#include <vector>
#include <iostream>

#define node_ptr std::shared_ptr<Node>

struct Node;

enum class NodeType {
	ID,
    NUMBER,
    STRING,
	BOOLEAN,
	OP,
	LIST,
	COMMA_LIST,
	PAREN,
	FUNC_CALL,
	ACCESSOR,
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
	node_ptr left;
	node_ptr right;
};

struct ListNode {
	std::vector<node_ptr> elements;
};

struct ParenNode {
	std::vector<node_ptr> elements;
};

struct FuncCallNode {
	std::string name;
	std::vector<node_ptr> args;
};

struct AccessorNode {
	node_ptr container;
	node_ptr accessor;
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
	ListNode List;
	ParenNode Paren;
	FuncCallNode FuncCall;
	AccessorNode Accessor;
};

std::string node_repr(node_ptr);