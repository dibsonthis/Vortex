#pragma once
#include <variant>
#include <iomanip>
#include "../Node/Node.hpp"

#define value_ptr std::shared_ptr<Value>

uint8_t* int_to_bytes(int& integer);

int bytes_to_int(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

enum OpCode {
    OP_RETURN,
    OP_LOAD_CONST,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_EQ_EQ,
    OP_NOT_EQ,
    OP_LT_EQ,
    OP_GT_EQ,
    OP_LT,
    OP_GT,
    OP_RANGE,
    OP_STORE_VAR,
    OP_LOAD,
    OP_LOAD_GLOBAL,
    OP_LOAD_CLOSURE,
    OP_SET,
    OP_POP,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    OP_POP_JUMP_IF_FALSE,
    OP_POP_JUMP_IF_TRUE,
    OP_JUMP,
    OP_JUMP_BACK,
    OP_EXIT,
    OP_BREAK,
    OP_CONTINUE,
    OP_BUILD_LIST,
    OP_ACCESSOR,
    OP_LEN,
    OP_CALL
};

enum ValueType {
    Number,
    String,
    Boolean,
    List,
    Function,
    Native,
    None
};

struct Value;

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<uint8_t> lines;
    std::vector<Value> constants;
};

struct FunctionObj {
    std::string name;
    int arity;
    int defaults;
    Chunk chunk;
    std::vector<int> closed_vars;
};

typedef Value (*NativeFunction)(std::vector<Value>& args);

struct NativeFunctionObj {
    NativeFunction function = nullptr;
};

struct Value {
    ValueType type;
    std::variant
    <
        double, 
        std::string, 
        bool, 
        std::shared_ptr<std::vector<Value>>,
        std::shared_ptr<FunctionObj>,
        std::shared_ptr<NativeFunctionObj>
    > value;

    Value() : type(None) {}
    Value(ValueType type) : type(type) {
        switch (type)
        {
            case Number:
                value = 0.0f;
                break;
            case String:
                value = "";
                break;
            case Boolean:
                value = false;
                break;
            case List:
                value = std::make_shared<std::vector<Value>>();
                break;
            case Function:
                value = std::make_shared<FunctionObj>();
                break;
            case Native:
                value = std::make_shared<NativeFunctionObj>();
                break;
            default:    
                break;
        }
    }

    double& get_number() {
        return std::get<double>(this->value);
    }
    std::string& get_string() {
        return std::get<std::string>(this->value);
    }
    bool& get_boolean() {
        return std::get<bool>(this->value);
    }

    std::shared_ptr<std::vector<Value>>& get_list() {
        return std::get<std::shared_ptr<std::vector<Value>>>(this->value);
    }

    std::shared_ptr<FunctionObj>& get_function() {
        return std::get<std::shared_ptr<FunctionObj>>(this->value);
    }

    std::shared_ptr<NativeFunctionObj>& get_native() {
        return std::get<std::shared_ptr<NativeFunctionObj>>(this->value);
    }

    bool is_number() {
        return type == Number;
    }
    bool is_string() {
        return type == String;
    }
    bool is_boolean() {
        return type == Boolean;
    }
    bool is_list() {
        return type == List;
    }
    bool is_function() {
        return type == Function;
    }
    bool is_native() {
        return type == Native;
    }
    bool is_none() {
        return type == None;
    }
};

Value new_val();
Value number_val(double value);
Value string_val(std::string value);
Value boolean_val(bool value);
Value list_val();
Value function_val();
Value native_val();
Value none_val();

void printValue(Value value);

void add_code(Chunk& chunk, uint8_t code, uint8_t line = 0);

void add_opcode(Chunk& chunk, uint8_t op, int operand, uint8_t line = 0);

int add_constant(Chunk& chunk, Value value);

void add_constant_code(Chunk& chunk, Value value, uint8_t line = 0);

void add_bytes(Chunk& chunk, Value value, uint8_t op, uint8_t line = 0);

void patch_bytes(Chunk& chunk, int offset, uint8_t* bytes);

static int simple_instruction(std::string name, int offset);

static int constant_instruction(std::string name, Chunk& chunk, int offset);
static int op_code_instruction(std::string name, Chunk& chunk, int offset);

int disassemble_instruction(Chunk& chunk, int offset);

void disassemble_chunk(Chunk& chunk, std::string name);