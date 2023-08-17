#include "Bytecode.hpp"

uint8_t* int_to_bytes(int& integer) {
    return static_cast<uint8_t*>(static_cast<void*>(&integer));
}

int bytes_to_int(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    return int(a | b << 8 | c << 16 | d << 24);
}

Value new_val() {
    return Value(None);
}

Value number_val(double value) {
    Value val(Number);
    val.value = value;
    return val;
}

Value string_val(std::string value) {
    Value val(String);
    val.value = value;
    return val;
}

Value boolean_val(bool value) {
    Value val(Boolean);
    val.value = value;
    return val;
}

Value list_val() {
    Value val(List);
    return val;
}

Value function_val() {
    Value val(Function);
    return val;
}

Value none_val() {
    Value val(None);
    return val;
}

void printValue(Value value) {
    switch(value.type) {
        case Number: {
            std::cout << std::fixed << value.get_number() << std::defaultfloat;
            break;
        }
        case String: {
            std::cout << value.get_string();
            break;
        }
        case Boolean: {
            std::cout << (value.get_boolean() ? "true" : "false");
            break;
        }
        case List: {
            std::cout << "[ ";
            for (Value& v : *value.get_list()) {
                printValue(v);
                std::cout << " ";
            }
            std::cout << "]";
            break;
        }
        case Function: {
            std::cout << "Function: " << value.get_function()->name;
            break;
        }
        case None: {
            std::cout << "None";
            break;
        }
        default: {
            std::cout << "Undefined";
        }
    }
}

void add_code(Chunk& chunk, uint8_t code, uint8_t line) {
    chunk.code.push_back(code);
    chunk.lines.push_back(line);
}

int add_constant(Chunk& chunk, Value value) {
    chunk.constants.push_back(value);
    return chunk.constants.size()-1;
}

void add_constant_code(Chunk& chunk, Value value, uint8_t line) {
    int constant = add_constant(chunk, value);
    auto const_bytes = int_to_bytes(constant);

    add_code(chunk, OP_LOAD_CONST, line);
    for (int i = 0; i < 4; i++) {
        add_code(chunk, const_bytes[i], line);
    }
}

void add_bytes(Chunk& chunk, Value value, uint8_t op, uint8_t line) {
    int constant = add_constant(chunk, value);
    auto const_bytes = int_to_bytes(constant);

    add_code(chunk, op, line);
    for (int i = 0; i < 4; i++) {
        add_code(chunk, const_bytes[i], line);
    }
}

void patch_bytes(Chunk& chunk, int offset, uint8_t* bytes) {
    for (int i = 0; i < 4; i++) {
        chunk.code[offset + i] = bytes[i];
    }
}

void add_opcode(Chunk& chunk, uint8_t op, int operand, uint8_t line) {
    auto bytes = int_to_bytes(operand);

    add_code(chunk, op, line);
    for (int i = 0; i < 4; i++) {
        add_code(chunk, bytes[i], line);
    }
}

static int simple_instruction(std::string name, int offset) {
    printf("%s\n", name.c_str());
    return offset + 1;
}

static int constant_instruction(std::string name, Chunk& chunk, int offset) {
    int constant = bytes_to_int(chunk.code[offset + 1], chunk.code[offset + 2], chunk.code[offset + 3], chunk.code[offset + 4]);
    printf("%-16s %4d '", name.c_str(), constant);
    printValue(chunk.constants[constant]);
    printf("'\n");
    return offset + 5;
}

static int op_code_instruction(std::string name, Chunk& chunk, int offset) {
    int operand = bytes_to_int(chunk.code[offset + 1], chunk.code[offset + 2], chunk.code[offset + 3], chunk.code[offset + 4]);
    printf("%-16s %4d", name.c_str(), operand);
    printf("\n");
    return offset + 5;
}

int disassemble_instruction(Chunk& chunk, int offset) {
    printf("%04d ", offset);

    if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk.lines[offset]);
    }

    uint8_t instruction = chunk.code[offset];
    switch (instruction) {
    case OP_EXIT:
        return simple_instruction("OP_EXIT", offset);
    case OP_POP:
        return simple_instruction("OP_POP", offset);
    case OP_RETURN:
        return simple_instruction("OP_RETURN", offset);
    case OP_LOAD_CONST:
        return constant_instruction("OP_LOAD_CONST", chunk, offset);
    case OP_STORE_VAR:
        return constant_instruction("OP_STORE_VAR", chunk, offset);
    case OP_LOAD:
        return op_code_instruction("OP_LOAD", chunk, offset);
    case OP_SET:
        return op_code_instruction("OP_SET", chunk, offset);
    case OP_JUMP_IF_FALSE:
        return op_code_instruction("OP_JUMP_IF_FALSE", chunk, offset);
    case OP_JUMP_IF_TRUE:
        return op_code_instruction("OP_JUMP_IF_TRUE", chunk, offset);
    case OP_POP_JUMP_IF_FALSE:
        return op_code_instruction("OP_POP_JUMP_IF_FALSE", chunk, offset);
    case OP_POP_JUMP_IF_TRUE:
        return op_code_instruction("OP_POP_JUMP_IF_TRUE", chunk, offset);
    case OP_JUMP:
        return op_code_instruction("OP_JUMP", chunk, offset);
    case OP_JUMP_BACK:
        return op_code_instruction("OP_JUMP_BACK", chunk, offset);
    case OP_BREAK:
        return simple_instruction("OP_BREAK", offset);
    case OP_CONTINUE:
        return simple_instruction("OP_CONTINUE", offset);
    case OP_BUILD_LIST:
        return op_code_instruction("OP_BUILD_LIST", chunk, offset);
    case OP_CALL:
        return op_code_instruction("OP_CALL", chunk, offset);
    case OP_NEGATE:
        return simple_instruction("OP_NEGATE", offset);
    case OP_ADD:
        return simple_instruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return simple_instruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return simple_instruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return simple_instruction("OP_DIVIDE", offset);
    case OP_NOT:
        return simple_instruction("OP_NOT", offset);
    case OP_EQ_EQ:
        return simple_instruction("OP_EQ_EQ", offset);
    case OP_NOT_EQ:
        return simple_instruction("OP_NOT_EQ", offset);
    case OP_LT_EQ:
        return simple_instruction("OP_LT_EQ", offset);
    case OP_GT_EQ:
        return simple_instruction("OP_GT_EQ", offset);
    case OP_LT:
        return simple_instruction("OP_LT", offset);
    case OP_GT:
        return simple_instruction("OP_GT", offset);
    case OP_RANGE:
        return simple_instruction("OP_RANGE", offset);
    case OP_ACCESSOR:
        return simple_instruction("OP_ACCESSOR", offset);
    case OP_LEN:
        return simple_instruction("OP_LEN", offset);
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}

void disassemble_chunk(Chunk& chunk, std::string name) {
    printf("== %s ==\n", name.c_str());

    for (int offset = 0; offset < chunk.code.size();) {
        offset = disassemble_instruction(chunk, offset);
    }
}