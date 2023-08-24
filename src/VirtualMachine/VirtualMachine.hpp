#pragma once

#include <iostream>
#include <sstream>

#include "../utils/utils.hpp"
#include "../Lexer/Lexer.hpp"
#include "../Parser/Parser.hpp"
#include "../Bytecode/Bytecode.hpp"
#include "../Bytecode/Generator.hpp"

// #define DEBUG_TRACE_EXECUTION

struct Closure {
  int index;
  std::shared_ptr<Value> value;
};

struct CallFrame {
  std::shared_ptr<FunctionObj> function;
  uint8_t* ip;
  int frame_start;
  int sp;
  int instruction_index;
};
struct VM {
    std::vector<Value> stack;
    Value* sp;
    /* Possibly change to array with specified MAX_DEPTH */
    std::vector<CallFrame> frames;
    std::vector<Value*> objects;
    std::unordered_map<std::string, Value> globals;
};

enum EvaluateResult {
  EVALUATE_OK,
  EVALUATE_COMPILE_ERROR,
  EVALUATE_RUNTIME_ERROR
};

void push(VM& vm, Value& value);
Value pop(VM& vm);

static void runtimeError(VM& vm, std::string message, ...);
static void define_global(VM& vm, std::string name, Value value);
static void define_native(VM& vm, std::string name, NativeFunction function);
static EvaluateResult run(VM& vm);
EvaluateResult evaluate(VM& vm);

void freeVM(VM& vm);

bool is_equal(Value& v1, Value& v2);
bool is_falsey(Value& value);

static Value printNative(std::vector<Value>& args);
static Value clockNative(std::vector<Value>& args);
static Value disNative(std::vector<Value>& args);
static Value toStringNative(std::vector<Value>& args);
static Value lengthNative(std::vector<Value>& args);