#pragma once

#include "../Bytecode/Bytecode.hpp"

// #define DEBUG_TRACE_EXECUTION

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

void push(VM& vm, Value& value);
Value pop(VM& vm);

static void runtimeError(VM& vm, std::string message, ...);
static EvaluateResult run(VM& vm);
EvaluateResult evaluate(VM& vm, Chunk& chunk);