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
	OBJECT,
	OBJECT_DECONSTRUCT,
	COMMA_LIST,
	PAREN,
	FUNC_CALL,
	FUNC,
	INTERFACE,
	TRAIT,
	TRAIT_IMPL,
	ACCESSOR,
	RETURN,
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

struct ObjectNode {
	std::vector<node_ptr> elements;
};

struct ObjectDeconstructNode {
	std::string name;
	std::vector<node_ptr> elements;
};

struct ParenNode {
	std::vector<node_ptr> elements;
};

struct FuncCallNode {
	std::string name;
	std::vector<node_ptr> args;
};

struct FuncNode {
	std::string name;
	node_ptr params;
	node_ptr body;
};

struct AccessorNode {
	node_ptr container;
	node_ptr accessor;
};

struct InterfaceNode {
	std::string name;
	std::vector<node_ptr> elements;
};

struct TraitNode {
	std::string name;
	std::vector<node_ptr> elements;
};

struct TraitImplNode {
	node_ptr implementor;
	std::string name;
	std::vector<node_ptr> elements;
};

struct ReturnNode {
	node_ptr value;
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
	ObjectNode Object;
	ParenNode Paren;
	FuncCallNode FuncCall;
	FuncNode Function;
	AccessorNode Accessor;
	InterfaceNode Interface;
	TraitNode Trait;
	TraitImplNode TraitImplementation;
	ReturnNode Return;
	ObjectDeconstructNode ObjectDeconstruct;
};

std::string node_repr(node_ptr);