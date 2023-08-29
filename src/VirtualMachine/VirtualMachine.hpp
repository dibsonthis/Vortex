#pragma once

#include <iostream>
#include <sstream>

#include "../utils/utils.hpp"
#include "../Lexer/Lexer.hpp"
#include "../Parser/Parser.hpp"
#include "../Bytecode/Bytecode.hpp"
#include "../Bytecode/Generator.hpp"

#define GCC_COMPILER (defined(__GNUC__) && !defined(__clang__))

#if GCC_COMPILER
    #if __apple__ || __linux__
        #include <dlfcn.h>
    #else
        #include <windows.h>
    #endif
#else
    #if defined(__APPLE__) || defined(__linux__)
        #include <dlfcn.h>
    #else
        #include <windows.h>
    #endif
#endif

// #define DEBUG_TRACE_EXECUTION

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
    int status = 0;
    std::vector<std::shared_ptr<Closure>> closed_values;

    VM() {
        stack.reserve(100000);
    }
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
Value copy(Value& value);

static Value print_builtin(std::vector<Value>& args);
static Value clock_builtin(std::vector<Value>& args);
static Value dis_builtin(std::vector<Value>& args);
static Value to_string_builtin(std::vector<Value>& args);
static Value to_number_builtin(std::vector<Value>& args);
static Value insert_builtin(std::vector<Value>& args);
static Value length_builtin(std::vector<Value>& args);
static Value load_lib_builtin(std::vector<Value>& args);
static Value copy_builtin(std::vector<Value>& args);
static Value sort_builtin(std::vector<Value>& args);
static Value info_builtin(std::vector<Value>& args);