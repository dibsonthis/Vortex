#include "VirtualMachine.hpp"

void push(VM& vm, Value& value) {
    vm.stack.push_back(value);
    vm.sp = &vm.stack.back();
}

Value pop(VM& vm) {
    Value value = std::move(vm.stack.back());
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
#define READ_INT() (bytes_to_int(READ_BYTE(), READ_BYTE(), READ_BYTE(), READ_BYTE()))
#define READ_CONSTANT() (vm.chunk.constants[READ_INT()])

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
            case OP_EXIT: {
                printValue(pop(vm));
                printf("\n");
                return EVALUATE_OK;
            }
            case OP_RETURN: {
                printValue(pop(vm));
                printf("\n");
                return EVALUATE_OK;
            }
            case OP_LOAD_CONST: {
                Value constant = READ_CONSTANT();
                push(vm, constant);
                break;
            }
            case OP_LOAD: {
                int index = READ_INT();
                push(vm, vm.stack[index]);
                break;
            }
            case OP_SET: {
                int index = READ_INT();
                vm.stack[index] = pop(vm);
                break;
            }
            case OP_JUMP_IF_FALSE: {
                int offset = READ_INT();
                if (is_falsey(vm.stack.back())) {
                    vm.ip += offset;
                }
                break;
            }
            case OP_JUMP_IF_TRUE: {
                int offset = READ_INT();
                if (!is_falsey(vm.stack.back())) {
                    vm.ip += offset;
                }
                break;
            }
            case OP_POP_JUMP_IF_FALSE: {
                int offset = READ_INT();
                if (is_falsey(vm.stack.back())) {
                    vm.ip += offset;
                }
                pop(vm);
                break;
            }
            case OP_POP_JUMP_IF_TRUE: {
                int offset = READ_INT();
                if (!is_falsey(vm.stack.back())) {
                    vm.ip += offset;
                }
                pop(vm);
                break;
            }
            case OP_JUMP: {
                int offset = READ_INT();
                vm.ip += offset;
                break;
            }
            case OP_JUMP_BACK: {
                int offset = READ_INT();
                vm.ip -= offset;
                break;
            }
            case OP_POP: {
                pop(vm);
                break;
            }
            case OP_BREAK: {
                while (*vm.ip != OP_JUMP_BACK) {
                    vm.ip++;
                }
                READ_BYTE();
                READ_INT();
                break;
            }
            case OP_CONTINUE: {
                while (*vm.ip != OP_JUMP_BACK) {
                    vm.ip++;
                }
                break;
            }
            case OP_BUILD_LIST: {
                Value list = list_val();
                int size = READ_INT();
                auto& list_val = list.get_list();
                for (int i = 0; i < size; i++) {
                    list_val->insert(list_val->begin(), std::make_shared<Value>(pop(vm)));
                }
                push(vm, list);
                break;
            }
            case OP_ACCESSOR: {
                Value _index = pop(vm);
                Value _list = pop(vm);

                if (!_list.is_list()) {
                    runtimeError(vm, "Object is not accessable");
                }
                if (!_index.is_number()) {
                    runtimeError(vm, "Accessor must be a number");
                }
                int index = _index.get_number();
                auto& list = _list.get_list();
                if (index >= list->size() || index < 0) {
                    Value none = none_val();
                    push(vm, none);
                } else {
                    push(vm, *((*list)[index]));
                }
                break;
            }
            case OP_LEN: {
                Value list = pop(vm);
                if (!list.is_list()) {
                    runtimeError(vm, "Operand must be a list");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = number_val(list.get_list()->size());
                push(vm, value);
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
                if (constant.is_none()) {
                    Value value = boolean_val(true);
                    push(vm, value);
                    break;
                }
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
                if (v1.is_string() && v2.is_string()) {
                    Value value = string_val(v1.get_string() + v2.get_string());
                    vm.objects.push_back(&value);
                    push(vm, value);
                    break;
                }
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
            case OP_EQ_EQ: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                Value value = boolean_val(is_equal(v1, v2));
                push(vm, value);
                break;
            }
            case OP_NOT_EQ: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                Value value = boolean_val(!is_equal(v1, v2));
                push(vm, value);
                break;
            }
            case OP_LT_EQ: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (!v1.is_number() || !v2.is_number() ) {
                    runtimeError(vm, "Operands must be numbers");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = boolean_val(v1.get_number() <= v2.get_number());
                push(vm, value);
                break;
            }
            case OP_GT_EQ: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (!v1.is_number() || !v2.is_number() ) {
                    runtimeError(vm, "Operands must be numbers");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = boolean_val(v1.get_number() >= v2.get_number());
                push(vm, value);
                break;
            }
            case OP_LT: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (!v1.is_number() || !v2.is_number() ) {
                    runtimeError(vm, "Operands must be numbers");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = boolean_val(v1.get_number() < v2.get_number());
                push(vm, value);
                break;
            }
            case OP_GT: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (!v1.is_number() || !v2.is_number() ) {
                    runtimeError(vm, "Operands must be numbers");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = boolean_val(v1.get_number() > v2.get_number());
                push(vm, value);
                break;
            }
            case OP_RANGE: {
                Value v2 = pop(vm);
                Value v1 = pop(vm);
                if (!v1.is_number() || !v2.is_number() ) {
                    runtimeError(vm, "Operands must be numbers");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value value = list_val();
                auto& list_value = value.get_list();
                for (int i = v1.get_number(); i < v2.get_number(); i++) {
                    list_value->push_back(std::make_shared<Value>(number_val(i)));
                }
                push(vm, value);
                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_INT
#undef READ_CONSTANT
}

EvaluateResult evaluate(VM& vm, Chunk& chunk) {
    vm.chunk = chunk;
    vm.ip = &vm.chunk.code[0];
    return run(vm);
}

bool is_equal(Value& v1, Value& v2) {
    if (v1.type != v2.type) {
        return false;
    }

    if (v1.is_number()) {
        return v1.get_number() == v2.get_number();
    }
    if (v1.is_string()) {
        return v1.get_string() == v2.get_string();
    }
    if (v1.is_boolean()) {
        return v1.get_boolean() == v2.get_boolean();
    }

    return false;
}

bool is_falsey(Value& value) {
    if (value.is_none() || value.is_boolean() && !value.get_boolean()) {
        return true;
    }

    return false;
}

void freeVM(VM& vm) {
    //
}