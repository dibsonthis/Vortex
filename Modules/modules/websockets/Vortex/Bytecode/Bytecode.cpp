#include "Bytecode.hpp"

uint8_t *int_to_bytes(int &integer)
{
    return static_cast<uint8_t *>(static_cast<void *>(&integer));
}

int bytes_to_int(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return int(a | b << 8 | c << 16 | d << 24);
}

Value new_val()
{
    return Value(None);
}

Value number_val(double value)
{
    Value val(Number);
    val.value = value;
    return val;
}

Value string_val(std::string value)
{
    Value val(String);
    val.value = value;
    return val;
}

Value boolean_val(bool value)
{
    Value val(Boolean);
    val.value = value;
    return val;
}

Value list_val()
{
    Value val(List);
    return val;
}

Value type_val(std::string name)
{
    Value val(Type);
    val.get_type()->name = name;
    return val;
}

Value object_val()
{
    Value val(Object);
    return val;
}

Value function_val()
{
    Value val(Function);
    return val;
}

Value native_val()
{
    Value val(Native);
    return val;
}

Value pointer_val()
{
    Value val(Pointer);
    return val;
}

Value none_val()
{
    Value val(None);
    return val;
}

std::string toString(Value value)
{
    switch (value.type)
    {
    case Number:
    {
        double number = value.get_number();
        if (std::floor(number) == number)
        {
            return std::to_string((long long)number);
        }
        char buff[100];
        snprintf(buff, sizeof(buff), "%.8g", number);
        std::string buffAsStdStr = buff;
        return buffAsStdStr;
        // return std::to_string(number);
    }
    case String:
    {
        return (value.get_string());
    }
    case Boolean:
    {
        return (value.get_boolean() ? "true" : "false");
    }
    case List:
    {
        std::string repr = "[";
        for (int i = 0; i < value.get_list()->size(); i++)
        {
            Value &v = value.get_list()->at(i);
            repr += toString(v);
            if (i < value.get_list()->size() - 1)
            {
                repr += ", ";
            }
        }
        repr += "]";
        return repr;
    }
    case Type:
    {
        return "Type: " + value.get_type()->name;
    }
    case Object:
    {
        auto &obj = value.get_object();
        std::string repr;
        if (obj->type)
        {
            repr += value.get_object()->type->name + " ";
        }
        repr += "{ ";
        int i = 0;
        int size = obj->values.size();
        for (std::string &key : obj->keys)
        {
            repr += key + ": " + toString(obj->values[key]);
            i++;
            if (i < size)
            {
                repr += ", ";
            }
        }
        repr += " }";
        return repr;
    }
    case Function:
    {
        return "Function: " + value.get_function()->name;
    }
    case Native:
    {
        return "Native Function: " + value.get_native()->name;
    }
    case Pointer:
    {
        return "<Pointer>";
    }
    case None:
    {
        return "None";
    }
    default:
    {
        return "Undefined";
    }
    }
}

void printValue(Value value)
{
    std::cout << toString(value) << std::flush;
}

void add_code(Chunk &chunk, uint8_t code, int line)
{
    chunk.code.push_back(code);
    chunk.lines.push_back(line);
}

int add_constant(Chunk &chunk, Value value)
{
    chunk.constants.push_back(value);
    return chunk.constants.size() - 1;
}

void add_constant_code(Chunk &chunk, Value value, int line)
{
    int constant = add_constant(chunk, value);
    auto const_bytes = int_to_bytes(constant);

    add_code(chunk, OP_LOAD_CONST, line);
    for (int i = 0; i < 4; i++)
    {
        add_code(chunk, const_bytes[i], line);
    }
}

void add_bytes(Chunk &chunk, Value value, uint8_t op, int line)
{
    int constant = add_constant(chunk, value);
    auto const_bytes = int_to_bytes(constant);

    add_code(chunk, op, line);
    for (int i = 0; i < 4; i++)
    {
        add_code(chunk, const_bytes[i], line);
    }
}

void patch_bytes(Chunk &chunk, int offset, uint8_t *bytes)
{
    for (int i = 0; i < 4; i++)
    {
        chunk.code[offset + i] = bytes[i];
    }
}

void add_opcode(Chunk &chunk, uint8_t op, int operand, int line)
{
    auto bytes = int_to_bytes(operand);

    add_code(chunk, op, line);
    for (int i = 0; i < 4; i++)
    {
        add_code(chunk, bytes[i], line);
    }
}

static int simple_instruction(std::string name, int offset)
{
    printf("%s\n", name.c_str());
    return offset + 1;
}

static int constant_instruction(std::string name, Chunk &chunk, int offset)
{
    int constant = bytes_to_int(chunk.code[offset + 1], chunk.code[offset + 2], chunk.code[offset + 3], chunk.code[offset + 4]);
    printf("%-16s %4d '", name.c_str(), constant);
    printValue(chunk.constants[constant]);
    printf("'\n");
    return offset + 5;
}

static int op_code_instruction(std::string name, Chunk &chunk, int offset)
{
    int operand = bytes_to_int(chunk.code[offset + 1], chunk.code[offset + 2], chunk.code[offset + 3], chunk.code[offset + 4]);
    printf("%-16s %4d", name.c_str(), operand);
    printf("\n");
    return offset + 5;
}

int disassemble_instruction(Chunk &chunk, int offset)
{
    printf("%04d ", offset);

    if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", chunk.lines[offset]);
    }

    uint8_t instruction = chunk.code[offset];
    switch (instruction)
    {
    case OP_EXIT:
        return simple_instruction("OP_EXIT", offset);
    case OP_POP:
        return simple_instruction("OP_POP", offset);
    case OP_POP_CLOSE:
        return simple_instruction("OP_POP_CLOSE", offset);
    case OP_RETURN:
        return simple_instruction("OP_RETURN", offset);
    case OP_YIELD:
        return simple_instruction("OP_YIELD", offset);
    case OP_LOAD_GLOBAL:
        return op_code_instruction("OP_LOAD_GLOBAL", chunk, offset);
    case OP_LOAD_CONST:
        return constant_instruction("OP_LOAD_CONST", chunk, offset);
    case OP_LOAD_THIS:
        return simple_instruction("OP_LOAD_THIS", offset);
    case OP_STORE_VAR:
        return constant_instruction("OP_STORE_VAR", chunk, offset);
    case OP_LOAD:
        return op_code_instruction("OP_LOAD", chunk, offset);
    case OP_LOAD_CLOSURE:
        return op_code_instruction("OP_LOAD_CLOSURE", chunk, offset);
    case OP_SET:
        return op_code_instruction("OP_SET", chunk, offset);
    case OP_SET_FORCE:
        return op_code_instruction("OP_SET_FORCE", chunk, offset);
    case OP_SET_PROPERTY:
        return simple_instruction("OP_SET_PROPERTY", offset);
    case OP_SET_CLOSURE:
        return op_code_instruction("OP_SET_CLOSURE", chunk, offset);
    case OP_MAKE_CLOSURE:
        return op_code_instruction("OP_MAKE_CLOSURE", chunk, offset);
    case OP_MAKE_TYPE:
        return op_code_instruction("OP_MAKE_TYPE", chunk, offset);
    case OP_MAKE_TYPED:
        return simple_instruction("OP_MAKE_TYPED", offset);
    case OP_MAKE_OBJECT:
        return op_code_instruction("OP_MAKE_OBJECT", chunk, offset);
    case OP_MAKE_FUNCTION:
        return op_code_instruction("OP_MAKE_FUNCTION", chunk, offset);
    case OP_HOOK_ONCHANGE:
        return op_code_instruction("OP_HOOK_ONCHANGE", chunk, offset);
    case OP_HOOK_CLOSURE_ONCHANGE:
        return op_code_instruction("OP_HOOK_CLOSURE_ONCHANGE", chunk, offset);
    case OP_MAKE_CONST:
        return simple_instruction("OP_MAKE_CONST", offset);
    case OP_MAKE_NON_CONST:
        return simple_instruction("OP_MAKE_NON_CONST", offset);
    case OP_TYPE_DEFAULTS:
        return op_code_instruction("OP_TYPE_DEFAULTS", chunk, offset);
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
    case OP_LOOP:
        return op_code_instruction("OP_LOOP", chunk, offset);
    case OP_LOOP_END:
        return simple_instruction("OP_LOOP_END", offset);
    case OP_ITER:
        return simple_instruction("OP_ITER", offset);
    case OP_BREAK:
        return simple_instruction("OP_BREAK", offset);
    case OP_CONTINUE:
        return simple_instruction("OP_CONTINUE", offset);
    case OP_BUILD_LIST:
        return op_code_instruction("OP_BUILD_LIST", chunk, offset);
    case OP_CALL:
        return op_code_instruction("OP_CALL", chunk, offset);
    case OP_CALL_METHOD:
        return op_code_instruction("OP_CALL_METHOD", chunk, offset);
    case OP_UNPACK:
        return simple_instruction("OP_UNPACK", offset);
    case OP_REMOVE_PUSH:
        return op_code_instruction("OP_REMOVE_PUSH", chunk, offset);
    case OP_SWAP_TOS:
        return simple_instruction("OP_SWAP_TOS", offset);
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
    case OP_MOD:
        return simple_instruction("OP_MOD", offset);
    case OP_POW:
        return simple_instruction("OP_POW", offset);
    case OP_AND:
        return simple_instruction("OP_AND", offset);
    case OP_OR:
        return simple_instruction("OP_OR", offset);
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
    case OP_DOT:
        return simple_instruction("OP_DOT", offset);
    case OP_ACCESSOR:
        return op_code_instruction("OP_ACCESSOR", chunk, offset);
    case OP_IMPORT:
        return op_code_instruction("OP_IMPORT", chunk, offset);
    case OP_LEN:
        return simple_instruction("OP_LEN", offset);
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}

void disassemble_chunk(Chunk &chunk, std::string name)
{
    printf("== %s ==\n", name.c_str());

    for (int offset = 0; offset < chunk.code.size();)
    {
        offset = disassemble_instruction(chunk, offset);
    }
}

int advance(Chunk &chunk, int offset)
{
    uint8_t instruction = chunk.code[offset];
    switch (instruction)
    {
    case OP_EXIT:
        return offset + 1;
    case OP_POP:
        return offset + 1;
    case OP_POP_CLOSE:
        return offset + 1;
    case OP_RETURN:
        return offset + 1;
    case OP_YIELD:
        return offset + 1;
    case OP_LOAD_GLOBAL:
        return offset + 5;
    case OP_LOAD_CONST:
        return offset + 5;
    case OP_LOAD_THIS:
        return offset + 1;
    case OP_STORE_VAR:
        return offset + 5;
    case OP_LOAD:
        return offset + 5;
    case OP_LOAD_CLOSURE:
        return offset + 5;
    case OP_SET:
        return offset + 5;
    case OP_SET_FORCE:
        return offset + 5;
    case OP_SET_PROPERTY:
        return offset + 1;
    case OP_SET_CLOSURE:
        return offset + 5;
    case OP_MAKE_CLOSURE:
        return offset + 5;
    case OP_MAKE_TYPE:
        return offset + 5;
    case OP_MAKE_TYPED:
        return offset + 1;
    case OP_MAKE_OBJECT:
        return offset + 5;
    case OP_MAKE_FUNCTION:
        return offset + 5;
    case OP_HOOK_ONCHANGE:
        return offset + 5;
    case OP_HOOK_CLOSURE_ONCHANGE:
        return offset + 5;
    case OP_MAKE_CONST:
        return offset + 1;
    case OP_MAKE_NON_CONST:
        return offset + 1;
    case OP_TYPE_DEFAULTS:
        return offset + 5;
    case OP_JUMP_IF_FALSE:
        return offset + 5;
    case OP_JUMP_IF_TRUE:
        return offset + 5;
    case OP_POP_JUMP_IF_FALSE:
        return offset + 5;
    case OP_POP_JUMP_IF_TRUE:
        return offset + 5;
    case OP_JUMP:
        return offset + 5;
    case OP_JUMP_BACK:
        return offset + 5;
    case OP_LOOP:
        return offset + 5;
    case OP_LOOP_END:
        return offset + 1;
    case OP_ITER:
        return offset + 1;
    case OP_BREAK:
        return offset + 1;
    case OP_CONTINUE:
        return offset + 1;
    case OP_BUILD_LIST:
        return offset + 5;
    case OP_CALL:
        return offset + 5;
    case OP_CALL_METHOD:
        return offset + 5;
    case OP_UNPACK:
        return offset + 1;
    case OP_REMOVE_PUSH:
        return offset + 5;
    case OP_SWAP_TOS:
        return offset + 1;
    case OP_NEGATE:
        return offset + 1;
    case OP_ADD:
        return offset + 1;
    case OP_SUBTRACT:
        return offset + 1;
    case OP_MULTIPLY:
        return offset + 1;
    case OP_DIVIDE:
        return offset + 1;
    case OP_MOD:
        return offset + 1;
    case OP_POW:
        return offset + 1;
    case OP_AND:
        return offset + 1;
    case OP_OR:
        return offset + 1;
    case OP_NOT:
        return offset + 1;
    case OP_EQ_EQ:
        return offset + 1;
    case OP_NOT_EQ:
        return offset + 1;
    case OP_LT_EQ:
        return offset + 1;
    case OP_GT_EQ:
        return offset + 1;
    case OP_LT:
        return offset + 1;
    case OP_GT:
        return offset + 1;
    case OP_RANGE:
        return offset + 1;
    case OP_DOT:
        return offset + 1;
    case OP_ACCESSOR:
        return offset + 5;
    case OP_IMPORT:
        return offset + 5;
    case OP_LEN:
        return offset + 1;
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}

std::vector<int> instruction_offsets(Chunk &chunk)
{
    std::vector<int> offsets;

    for (int offset = 0; offset < chunk.code.size();)
    {
        offset = advance(chunk, offset);
        offsets.push_back(offset);
    }

    return offsets;
}