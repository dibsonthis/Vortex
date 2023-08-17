#pragma once

#include "../Bytecode/Bytecode.hpp"
#include "../Bytecode/Generator.hpp"

// #define DEBUG_TRACE_EXECUTION

struct CallFrame {
  std::shared_ptr<FunctionObj> function;
  uint8_t* ip;
  int frame_start;
  int sp;
};
struct VM {
    std::vector<Value> stack;
    Value* sp;
    /* Possibly change to array with specified MAX_DEPTH */
    std::vector<CallFrame> frames;
    std::vector<Value*> objects;
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
EvaluateResult evaluate(VM& vm);

void freeVM(VM& vm);

bool is_equal(Value& v1, Value& v2);
bool is_falsey(Value& value);