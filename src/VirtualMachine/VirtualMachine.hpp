#pragma once

#include "../Bytecode/Bytecode.hpp"

//#define DEBUG_TRACE_EXECUTION

struct VM {
    Chunk chunk;
    uint8_t* ip;
    std::vector<Value> stack;
    Value* sp;
};

enum EvaluateResult {
  EVALUATE_OK,
  EVALUATE_COMPILE_ERROR,
  EVALUATE_RUNTIME_ERROR
};

void push(VM& vm, Value& value) {
    vm.stack.push_back(value);
    vm.sp = &vm.stack.back();
}

Value pop(VM& vm) {
    Value value = vm.stack.back();
    vm.stack.pop_back();
    vm.sp--;
    return value;
}

static EvaluateResult run(VM& vm) {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk.constants[bytes_to_int(READ_BYTE(), READ_BYTE(), READ_BYTE(), READ_BYTE())])

    for (;;) {
        #ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        printf("[ ");
        for (Value value : vm.stack) {
            printValue(value);
            printf(" ");
        }
        printf("]");
        printf("\n");
            disassemble_instruction(vm.chunk, (int)(size_t)(vm.ip - &vm.chunk.code[0]));
        #endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_RETURN: {
                printValue(pop(vm));
                printf("\n");
                return EVALUATE_OK;
            }
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(vm, constant);
                break;
            }
            case OP_NEGATE: {
                Value constant = pop(vm);
                if (constant.type != Number) {
                    return EVALUATE_COMPILE_ERROR;
                }
                Value value = number_val(-constant.get_float());
                push(vm, value);
                break;
            }
            case OP_ADD: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (v1.type != Number || v2.type != Number ) {
                    return EVALUATE_COMPILE_ERROR;
                }
                Value value = number_val(v1.get_float() + v2.get_float());
                push(vm, value);
                break;
            }
            case OP_SUBTRACT: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (v1.type != Number || v2.type != Number ) {
                    return EVALUATE_COMPILE_ERROR;
                }
                Value value = number_val(v1.get_float() - v2.get_float());
                push(vm, value);
                break;
            }
            case OP_MULTIPLY: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (v1.type != Number || v2.type != Number ) {
                    return EVALUATE_COMPILE_ERROR;
                }
                Value value = number_val(v1.get_float() * v2.get_float());
                push(vm, value);
                break;
            }
            case OP_DIVIDE: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (v1.type != Number || v2.type != Number ) {
                    return EVALUATE_COMPILE_ERROR;
                }
                Value value = number_val(v1.get_float() / v2.get_float());
                push(vm, value);
                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}

EvaluateResult evaluate(VM& vm, Chunk& chunk) {
    vm.chunk = chunk;
    vm.ip = &vm.chunk.code[0];
    return run(vm);
}