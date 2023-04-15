#pragma once

#include <vector>
#include <iostream>
#include <unordered_map>

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
	VARIABLE_DECLARATION,
	VARIABLE_DECLARATION_MULTIPLE,
	CONSTANT_DECLARATION,
	CONSTANT_DECLARATION_MULTIPLE,
	FOR_LOOP,
	WHILE_LOOP,
	RETURN,
	IMPORT,
	IF_STATEMENT,
	IF_BLOCK,
	START_OF_FILE,
	END_OF_FILE,
	NONE
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
	std::unordered_map<std::string, node_ptr> properties;
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
	node_ptr caller;
};

struct FuncNode {
	std::string name;
	std::vector<node_ptr> args;
	std::vector<node_ptr> params;
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

struct VariableDeclatationNode {
	std::string name;
	node_ptr value;
};

struct VariableDeclatationMultipleNode {
	std::vector<node_ptr> variable_declarations;
};

struct ConstantDeclatationNode {
	std::string name;
	node_ptr value;
};

struct ConstantDeclatationMultipleNode {
	std::vector<node_ptr> constant_declarations;
};

struct ForLoopNode {
	node_ptr start;
	node_ptr end;
	node_ptr index_name;
	node_ptr value_name;
	node_ptr body;
	node_ptr iterator;
};

struct WhileLoopNode {
	node_ptr condition;
	node_ptr body;
};

struct ImportNode {
	node_ptr module;
	node_ptr target;
};

struct IfStatementNode {
	node_ptr condition;
	node_ptr body;
};

struct IfBlockNode {
	std::vector<node_ptr> statements;
};

struct ReturnNode {
	node_ptr value;
};

struct MetaInformation {
	bool is_const;
};

struct Hooks {
	std::vector<node_ptr> onChange;
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
	VariableDeclatationNode VariableDeclaration;
	ConstantDeclatationNode ConstantDeclaration;
	VariableDeclatationMultipleNode VariableDeclarationMultiple;
	ConstantDeclatationMultipleNode ConstantDeclarationMultiple;
	ForLoopNode ForLoop;
	WhileLoopNode WhileLoop;
	ObjectDeconstructNode ObjectDeconstruct;
	ImportNode Import;
	IfStatementNode IfStatement;
	IfBlockNode IfBlock;
	ReturnNode Return;
	MetaInformation Meta;
	Hooks Hooks;
};

std::string node_repr(node_ptr);