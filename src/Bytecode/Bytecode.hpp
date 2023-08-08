#pragma once
#include <variant>
#include "../Node/Node.hpp"

uint8_t* int_to_bytes(int& integer);

int bytes_to_int(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

enum OpCode {
    OP_RETURN,
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT
};

enum ValueType {
    Number,
    String,
    Boolean
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
};

Value number_val(float value);

Value string_val(std::string value);

Value boolean_val(bool value);

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<uint8_t> lines;
    std::vector<Value> constants;
};

void printValue(Value value);

void add_code(Chunk& chunk, uint8_t code, uint8_t line = 0);

int add_constant(Chunk& chunk, Value value);

void add_constant_code(Chunk& chunk, Value value, uint8_t line = 0);

static int simple_instruction(std::string name, int offset);

static int constant_instruction(std::string name, Chunk& chunk, int offset);

int disassemble_instruction(Chunk& chunk, int offset);

void disassemble_chunk(Chunk& chunk, std::string name);