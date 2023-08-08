#include "VirtualMachine.hpp"

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

static void runtimeError(VM& vm, std::string message, ...) {
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message.c_str(), args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - &vm.chunk.code[0] - 1;
    int line = vm.chunk.lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
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
                if (!constant.is_number()) {
                    runtimeError(vm, "Operand must be a number");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = number_val(-constant.get_number());
                push(vm, value);
                break;
            }
            case OP_NOT: {
                Value constant = pop(vm);
                if (!constant.is_boolean()) {
                    runtimeError(vm, "Operand must be a boolean");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = boolean_val(!constant.get_boolean());
                push(vm, value);
                break;
            }
            case OP_ADD: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (!v1.is_number() || !v2.is_number() ) {
                    runtimeError(vm, "Operands must be numbers");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = number_val(v1.get_number() + v2.get_number());
                push(vm, value);
                break;
            }
            case OP_SUBTRACT: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (!v1.is_number() || !v2.is_number() ) {
                    runtimeError(vm, "Operands must be numbers");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = number_val(v1.get_number() - v2.get_number());
                push(vm, value);
                break;
            }
            case OP_MULTIPLY: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (!v1.is_number() || !v2.is_number() ) {
                    runtimeError(vm, "Operands must be numbers");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = number_val(v1.get_number() * v2.get_number());
                push(vm, value);
                break;
            }
            case OP_DIVIDE: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (!v1.is_number() || !v2.is_number() ) {
                    runtimeError(vm, "Operands must be numbers");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = number_val(v1.get_number() / v2.get_number());
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