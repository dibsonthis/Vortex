#pragma once

#include <vector>
#include <iostream>
#include <unordered_map>
#include <memory>

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
	ENUM,
	HOOK,
	TYPE,
	TYPE_EXT,
	ACCESSOR,
	VARIABLE_DECLARATION,
	CONSTANT_DECLARATION,
	FOR_LOOP,
	WHILE_LOOP,
	RETURN,
	BREAK,
	CONTINUE,
	IMPORT,
	IF_STATEMENT,
	IF_BLOCK,
	START_OF_FILE,
	END_OF_FILE,
	NONE,
	LIB,
	POINTER,
	ANY,
	UNION,
	TRY_CATCH,
	ERROR,
	REF
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
    bool value = false;
};

struct OpNode {
	std::string value;
	node_ptr left;
	node_ptr right;
};

struct ListNode {
	std::vector<node_ptr> elements;
	bool is_union = false;
};

struct ObjectNode {
	std::vector<node_ptr> elements;
	std::unordered_map<std::string, node_ptr> properties;
	std::unordered_map<std::string, node_ptr> defaults;
	std::vector<std::string> keys;
	std::vector<node_ptr> values;
	bool is_enum = false;
};

struct RefNode {
	node_ptr value;
};

struct ObjectDeconstructNode {
	std::string name;
	node_ptr body;
};

struct ParenNode {
	std::vector<node_ptr> elements;
};

struct FuncCallNode {
	std::string name;
	std::vector<node_ptr> args;
	node_ptr caller;
	node_ptr inline_func;
};

struct FuncNode {
	std::string name;
	std::vector<node_ptr> args;
	std::vector<node_ptr> params;
	node_ptr body;
	bool is_hook = false;
	std::unordered_map<std::string, node_ptr> closure;
	std::string decl_filename;
	std::unordered_map<std::string, node_ptr> param_types;
	node_ptr return_type;
	std::vector<node_ptr> dispatch_functions;
};

struct AccessorNode {
	node_ptr container;
	node_ptr accessor;
};
struct TypeNode {
	std::string name;
	node_ptr body;
	node_ptr expr;
};

struct TypeExtNode {
	node_ptr type;
	node_ptr body;
};

struct EnumNode {
	std::string name;
	node_ptr body;
};

struct UnionNode {
	std::string name;
	node_ptr body;
};

struct TraitImplNode {
	node_ptr implementor;
	std::string name;
	std::vector<node_ptr> elements;
};

struct HookNode {
	std::string hook_name;
	std::string name;
	node_ptr function;
};

struct VariableDeclatationNode {
	std::string name;
	node_ptr value;
};

struct ConstantDeclatationNode {
	std::string name;
	node_ptr value;
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
	bool is_default = false;
};

struct IfStatementNode {
	node_ptr condition;
	node_ptr body;
};

struct IfBlockNode {
	std::vector<node_ptr> statements;
};

struct TryCatchNode {
	node_ptr try_body;
	node_ptr catch_keyword;
	node_ptr catch_body;
};


struct ReturnNode {
	node_ptr value;
};

struct MetaInformation {
	bool is_const = false;
	bool is_untyped_property = false;
	bool evaluated = false;
	int ref_count = 1;
};

struct Hooks {
	std::vector<node_ptr> onChange;
	std::vector<node_ptr> onCall;
};

struct LibNode {
	typedef node_ptr (*call_function_t)(std::string name, std::vector<node_ptr> args);
	void* handle;
	call_function_t call_function;
};

struct PointerNode {
	void* value;
};

struct TypeInfoNode {
	node_ptr type;
	std::string type_name;
	bool is_type = false;
	bool is_refinement_type = false;
	bool is_literal_type = false;
	bool is_general_type = false;
	bool is_decl = false;
};

struct ErrorNode {
	std::string message;
};

using _NodeType = std::variant<
	IdNode,
	NumberNode,
	StringNode,
	BooleanNode,
	OpNode,
	ListNode,
	ObjectNode,
	ParenNode,
	FuncCallNode,
	FuncNode,
	AccessorNode,
	TypeNode,
	HookNode,
	VariableDeclatationNode,
	ConstantDeclatationNode,
	ForLoopNode,
	WhileLoopNode,
	ObjectDeconstructNode,
	ImportNode,
	IfStatementNode,
	IfBlockNode,
	ReturnNode,
	LibNode,
	PointerNode,
	EnumNode,
	UnionNode,
	TypeExtNode,
	TryCatchNode,
	ErrorNode,
	RefNode
	>;

struct V : public _NodeType {

		using _NodeType::operator=;

		IdNode& ID() {
			return std::get<IdNode>(*this);
		}
		NumberNode& Number() {
			return std::get<NumberNode>(*this);
		}
		StringNode& String() {
			return std::get<StringNode>(*this);
		}
		BooleanNode& Boolean() {
			return std::get<BooleanNode>(*this);
		}
		OpNode& Op() {
			return std::get<OpNode>(*this);
		}
		ListNode& List() {
			return std::get<ListNode>(*this);
		}
		ObjectNode& Object() {
			return std::get<ObjectNode>(*this);
		}
		ParenNode& Paren() {
			return std::get<ParenNode>(*this);
		}
		FuncCallNode& FunctionCall() {
			return std::get<FuncCallNode>(*this);
		}
		FuncNode& Function() {
			return std::get<FuncNode>(*this);
		}
		AccessorNode& Accessor() {
			return std::get<AccessorNode>(*this);
		}
		TypeNode& Type() {
			return std::get<TypeNode>(*this);
		}
		HookNode& Hook() {
			return std::get<HookNode>(*this);
		}
		VariableDeclatationNode& VariableDeclaration() {
			return std::get<VariableDeclatationNode>(*this);
		}
		ConstantDeclatationNode& ConstantDeclatation() {
			return std::get<ConstantDeclatationNode>(*this);
		}
		ForLoopNode& ForLoop() {
			return std::get<ForLoopNode>(*this);
		}
		WhileLoopNode& WhileLoop() {
			return std::get<WhileLoopNode>(*this);
		}
		ObjectDeconstructNode& ObjectDeconstruct() {
			return std::get<ObjectDeconstructNode>(*this);
		}
		ImportNode& Import() {
			return std::get<ImportNode>(*this);
		}
		IfStatementNode& IfStatement() {
			return std::get<IfStatementNode>(*this);
		}
		IfBlockNode& IfBlock() {
			return std::get<IfBlockNode>(*this);
		}
		ReturnNode& Return() {
			return std::get<ReturnNode>(*this);
		}
		LibNode& Lib() {
			return std::get<LibNode>(*this);
		}
		PointerNode& Pointer() {
			return std::get<PointerNode>(*this);
		}
		EnumNode& Enum() {
			return std::get<EnumNode>(*this);
		}
		UnionNode& Union() {
			return std::get<UnionNode>(*this);
		}
		TypeExtNode& TypeExt() {
			return std::get<TypeExtNode>(*this);
		}
		TryCatchNode& TryCatch() {
			return std::get<TryCatchNode>(*this);
		}
		ErrorNode& Error() {
			return std::get<ErrorNode>(*this);
		}
		RefNode& Ref() {
			return std::get<RefNode>(*this);
		}
	};

struct Node {
	~Node() {
		Meta.ref_count--;
	}
	Node() = default;
    Node(NodeType type) : type(type) {
		switch(type) {
			case NodeType::ID: {
				_Node = IdNode();
				break;
			}
			case NodeType::STRING: {
				_Node = StringNode();
				break;
			}
			case NodeType::NUMBER: {
				_Node = NumberNode();
				break;
			}
			case NodeType::BOOLEAN: {
				_Node = BooleanNode();
				break;
			}
			case NodeType::OP: {
				_Node = OpNode();
				break;
			}
			case NodeType::LIST: {
				_Node = ListNode();
				break;
			}
			case NodeType::OBJECT: {
				_Node = ObjectNode();
				break;
			}
			case NodeType::PAREN: {
				_Node = ParenNode();
				break;
			}
			case NodeType::FUNC: {
				_Node = FuncNode();
				break;
			}
			case NodeType::FUNC_CALL: {
				_Node = FuncCallNode();
				break;
			}
			case NodeType::ACCESSOR: {
				_Node = AccessorNode();
				break;
			}
			case NodeType::TYPE: {
				_Node = TypeNode();
				break;
			}
			case NodeType::HOOK: {
				_Node = HookNode();
				break;
			}
			case NodeType::VARIABLE_DECLARATION: {
				_Node = VariableDeclatationNode();
				break;
			}
			case NodeType::CONSTANT_DECLARATION: {
				_Node = ConstantDeclatationNode();
				break;
			}
			case NodeType::FOR_LOOP: {
				_Node = ForLoopNode();
				break;
			}
			case NodeType::WHILE_LOOP: {
				_Node = WhileLoopNode();
				break;
			}
			case NodeType::OBJECT_DECONSTRUCT: {
				_Node = ObjectDeconstructNode();
				break;
			}
			case NodeType::IMPORT: {
				_Node = ImportNode();
				break;
			}
			case NodeType::IF_STATEMENT: {
				_Node = IfStatementNode();
				break;
			}
			case NodeType::IF_BLOCK: {
				_Node = IfBlockNode();
				break;
			}
			case NodeType::RETURN: {
				_Node = ReturnNode();
				break;
			}
			case NodeType::LIB: {
				_Node = LibNode();
				break;
			}
			case NodeType::POINTER: {
				_Node = PointerNode();
				break;
			}
			case NodeType::ENUM: {
				_Node = EnumNode();
				break;
			}
			case NodeType::UNION: {
				_Node = UnionNode();
				break;
			}
			case NodeType::TYPE_EXT: {
				_Node = TypeExtNode();
				break;
			}
			case NodeType::TRY_CATCH: {
				_Node = TryCatchNode();
				break;
			}
			case NodeType::ERROR: {
				_Node = ErrorNode();
				break;
			}
			case NodeType::REF: {
				_Node = RefNode();
				break;
			}
			default: {
				_Node = IdNode();
			}
		}
	}
	
	Node(int line, int column) : line(line), column(column) {}
	Node(NodeType type, int line, int column) : type(type), line(line), column(column) {
		switch(type) {
			case NodeType::ID: {
				_Node = IdNode();
				break;
			}
			case NodeType::STRING: {
				_Node = StringNode();
				break;
			}
			case NodeType::NUMBER: {
				_Node = NumberNode();
				break;
			}
			case NodeType::BOOLEAN: {
				_Node = BooleanNode();
				break;
			}
			case NodeType::OP: {
				_Node = OpNode();
				break;
			}
			case NodeType::LIST: {
				_Node = ListNode();
				break;
			}
			case NodeType::OBJECT: {
				_Node = ObjectNode();
				break;
			}
			case NodeType::PAREN: {
				_Node = ParenNode();
				break;
			}
			case NodeType::FUNC: {
				_Node = FuncNode();
				break;
			}
			case NodeType::FUNC_CALL: {
				_Node = FuncCallNode();
				break;
			}
			case NodeType::ACCESSOR: {
				_Node = AccessorNode();
				break;
			}
			case NodeType::TYPE: {
				_Node = TypeNode();
				break;
			}
			case NodeType::HOOK: {
				_Node = HookNode();
				break;
			}
			case NodeType::VARIABLE_DECLARATION: {
				_Node = VariableDeclatationNode();
				break;
			}
			case NodeType::CONSTANT_DECLARATION: {
				_Node = ConstantDeclatationNode();
				break;
			}
			case NodeType::FOR_LOOP: {
				_Node = ForLoopNode();
				break;
			}
			case NodeType::WHILE_LOOP: {
				_Node = WhileLoopNode();
				break;
			}
			case NodeType::OBJECT_DECONSTRUCT: {
				_Node = ObjectDeconstructNode();
				break;
			}
			case NodeType::IMPORT: {
				_Node = ImportNode();
				break;
			}
			case NodeType::IF_STATEMENT: {
				_Node = IfStatementNode();
				break;
			}
			case NodeType::IF_BLOCK: {
				_Node = IfBlockNode();
				break;
			}
			case NodeType::RETURN: {
				_Node = ReturnNode();
				break;
			}
			case NodeType::LIB: {
				_Node = LibNode();
				break;
			}
			case NodeType::POINTER: {
				_Node = PointerNode();
				break;
			}
			case NodeType::ENUM: {
				_Node = EnumNode();
				break;
			}
			case NodeType::UNION: {
				_Node = UnionNode();
				break;
			}
			case NodeType::TYPE_EXT: {
				_Node = TypeExtNode();
				break;
			}
			case NodeType::TRY_CATCH: {
				_Node = TryCatchNode();
				break;
			}
			case NodeType::ERROR: {
				_Node = ErrorNode();
				break;
			}
			case NodeType::REF: {
				_Node = RefNode();
				break;
			}
			default: {
				_Node = IdNode();
			}
		}
	}

    NodeType type;
	int column = 1;
	int line = 1;

	V _Node = {};

	MetaInformation Meta;
	Hooks Hooks;
	TypeInfoNode TypeInfo;
};

std::string node_repr(node_ptr);