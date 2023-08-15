#pragma once
#include <variant>
#include "../Node/Node.hpp"

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
    OP_STORE_VAR,
    OP_LOAD,
    OP_SET,
    OP_POP,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    OP_POP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_JUMP_BACK,
    OP_EXIT,
    OP_BREAK,
    OP_CONTINUE
};

enum ValueType {
    Number,
    String,
    Boolean,
    None
};

struct Value {
    ValueType type;
    std::variant<float, std::string, bool> value;
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
            default:    
                break;
        }
    }
    float get_number() {
        return std::get<float>(this->value);
    }
    std::string get_string() {
        return std::get<std::string>(this->value);
    }
    bool get_boolean() {
        return std::get<bool>(this->value);
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
    bool is_none() {
        return type == None;
    }
};

Value number_val(float value);
Value string_val(std::string value);
Value boolean_val(bool value);
Value none_val();

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<uint8_t> lines;
    std::vector<Value> constants;
};

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