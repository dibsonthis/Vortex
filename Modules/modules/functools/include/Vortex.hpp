#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <variant>
#include <cstdarg>
#include <cmath>

enum class ValueType
{
    Number,
    String,
    Boolean,
    List,
    Type,
    Object,
    Function,
    Native,
    Pointer,
    None
};

struct Value;
struct Closure;

std::string toString(Value value);

struct Chunk
{
    std::vector<uint8_t> code;
    std::vector<int> lines;
    std::vector<Value> constants;
    std::vector<std::string> variables;
    std::vector<std::string> public_variables;
    std::string import_path;
};

struct ClosedVar
{
    std::string name;
    int index;
    bool is_local;
};

struct FunctionObj
{
    std::string name;
    int arity;
    int defaults;
    Chunk chunk;
    std::vector<int> instruction_offsets;
    std::vector<ClosedVar> closed_var_indexes;
    std::vector<std::shared_ptr<Closure>> closed_vars;
    std::vector<Value> default_values;
    std::shared_ptr<Value> object;
    std::vector<std::string> params;
    bool is_generator = false;
    bool generator_init = false;
    bool generator_done = false;
    bool is_type_generator = false;
    std::string import_path;
};

struct TypeObj
{
    std::string name;
    std::unordered_map<std::string, Value> types;
    std::unordered_map<std::string, Value> defaults;
};

struct ObjectObj
{
    std::shared_ptr<TypeObj> type;
    std::unordered_map<std::string, Value> values;
    std::vector<std::string> keys;
    std::string type_name;
};

struct PointerObj
{
    void *value;
};

typedef Value (*NativeFunction)(std::vector<Value> &args);

struct NativeFunctionObj
{
    std::string name;
    NativeFunction function = nullptr;
};

struct Meta
{
    bool unpack = false;
    bool packer = false;
    bool is_const = false;
    bool temp_non_const = false;
};

struct ValueHooks
{
    std::shared_ptr<Value> onChangeHook = nullptr;
std::string onChangeHookName;
};

static long long int v_counter = 0;
struct Value
{
    ValueType type;
    Meta meta;
    ValueHooks hooks;
long long int id;
    std::variant<
        double,
        std::string,
        bool,
        std::shared_ptr<std::vector<Value>>,
        std::shared_ptr<FunctionObj>,
        std::shared_ptr<TypeObj>,
        std::shared_ptr<ObjectObj>,
        std::shared_ptr<NativeFunctionObj>,
        std::shared_ptr<PointerObj>>
        value;

    Value() : type(None)
    {
        id = ++v_counter;
    }
    Value(ValueType type) : type(type)
    {
        id = ++v_counter;
        switch (type)
        {
        case ValueType::Number:
            value = 0.0f;
            break;
        case ValueType::String:
            value = "";
            break;
        case ValueType::Boolean:
            value = false;
            break;
        case ValueType::List:
            value = std::make_shared<std::vector<Value>>();
            break;
        case ValueType::Type:
            value = std::make_shared<TypeObj>();
            break;
        case ValueType::Object:
            value = std::make_shared<ObjectObj>();
            break;
        case ValueType::Function:
            value = std::make_shared<FunctionObj>();
            break;
        case ValueType::Native:
            value = std::make_shared<NativeFunctionObj>();
            break;
        case ValueType::Pointer:
            value = std::make_shared<PointerObj>();
            break;
        default:
            break;
        }
    }

    double &get_number()
    {
        return std::get<double>(this->value);
    }
    std::string &get_string()
    {
        return std::get<std::string>(this->value);
    }
    bool &get_boolean()
    {
        return std::get<bool>(this->value);
    }

    std::shared_ptr<std::vector<Value>> &get_list()
    {
        return std::get<std::shared_ptr<std::vector<Value>>>(this->value);
    }

    std::shared_ptr<TypeObj> &get_type()
    {
        return std::get<std::shared_ptr<TypeObj>>(this->value);
    }

    std::shared_ptr<ObjectObj> &get_object()
    {
        return std::get<std::shared_ptr<ObjectObj>>(this->value);
    }

    std::shared_ptr<FunctionObj> &get_function()
    {
        return std::get<std::shared_ptr<FunctionObj>>(this->value);
    }

    std::shared_ptr<NativeFunctionObj> &get_native()
    {
        return std::get<std::shared_ptr<NativeFunctionObj>>(this->value);
    }

    std::shared_ptr<PointerObj> &get_pointer()
    {
        return std::get<std::shared_ptr<PointerObj>>(this->value);
    }

    bool is_number()
    {
        return type == ValueType::Number;
    }
    bool is_string()
    {
        return type == ValueType::String;
    }
    bool is_boolean()
    {
        return type == ValueType::Boolean;
    }
    bool is_list()
    {
        return type == ValueType::List;
    }
    bool is_type()
    {
        return type == ValueType::Type;
    }
    bool is_object()
    {
        return type == ValueType::Object;
    }
    bool is_function()
    {
        return type == ValueType::Function;
    }
    bool is_native()
    {
        return type == ValueType::Native;
    }
    bool is_pointer()
    {
        return type == ValueType::Pointer;
    }
    bool is_none()
    {
        return type == ValueType::None;
    }
    std::string type_repr()
    {
        switch (type)
        {
        case ValueType::Number:
            return "Number";
        case ValueType::String:
            return "String";
        case ValueType::Boolean:
            return "Boolean";
        case ValueType::List:
            return "List";
        case ValueType::Object:
            return "Object";
        case ValueType::Function:
            return "Function";
        case ValueType::Native:
            return "Native";
        case ValueType::Pointer:
            return "Pointer";
        case ValueType::Type:
            return "Type";
        case ValueType::None:
            return "None";
        default:
            return "Unknown";
        }
    }
    std::string value_repr()
    {
        return toString(*this);
    }
};

struct Closure
{
    std::string name;
    std::string frame_name;
    bool is_local;
    int index;
    Value *location;
    Value closed;
    Value *initial_location;
};

Value new_val()
{
    return Value(ValueType::None);
}

Value number_val(double value)
{
    Value val(ValueType::Number);
    val.value = value;
    return val;
}

Value string_val(std::string value)
{
    Value val(ValueType::String);
    val.value = value;
    return val;
}

Value boolean_val(bool value)
{
    Value val(ValueType::Boolean);
    val.value = value;
    return val;
}

Value list_val()
{
    Value val(ValueType::List);
    return val;
}

Value type_val(std::string name)
{
    Value val(ValueType::Type);
    val.get_type()->name = name;
    return val;
}

Value object_val()
{
    Value val(ValueType::Object);
    return val;
}

Value function_val()
{
    Value val(ValueType::Function);
    return val;
}

Value native_val()
{
    Value val(ValueType::Native);
    return val;
}

Value pointer_val()
{
    Value val(ValueType::Pointer);
    return val;
}

Value none_val()
{
    Value val(ValueType::None);
    return val;
}

void error(std::string message)
{
    std::cout << message << "\n";
    exit(1);
}

struct CallFrame
{
    std::string name;
    std::shared_ptr<FunctionObj> function;
    uint8_t *ip;
    int frame_start;
    int sp;
    int instruction_index;
    std::vector<Value> gen_stack;
};
struct CachedImport
{
    Value import_object;
    std::unordered_map<std::string, Value> import_globals;
};
struct VM
{
    std::vector<Value> stack;
    Value *sp;
    /* Possibly change to array with specified MAX_DEPTH */
    std::vector<CallFrame> frames;
    std::unordered_map<std::string, std::shared_ptr<CallFrame>> gen_frames;
    std::vector<Value *> objects;
    std::unordered_map<std::string, Value> globals;
    int status = 0;
    std::vector<std::shared_ptr<Closure>> closed_values;
    int coro_count = 0;
    std::vector<int> try_instructions;
int call_stack_limit = 3000;
std::unordered_map<std::string, CachedImport> import_cache;

    VM()
    {
        stack.reserve(100000);
    }
};

Value error_object(std::string message, std::string error_type = "GenericError")
{
    Value error_obj = object_val();
    error_obj.get_object()->type_name = "Error";
    error_obj.get_object()->keys = {"message", "type"};
    error_obj.get_object()->values["message"] = string_val(message);
    error_obj.get_object()->values["type"] = string_val(error_type);

    return error_obj;
}

static void runtimeError(VM &vm, std::string message, ...)
{

    vm.status = 1;

    CallFrame frame = vm.frames.back();

    va_list args;
    va_start(args, message);
    vfprintf(stderr, message.c_str(), args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frames.size() - 1; i >= 0; i--)
    {
        CallFrame *frame = &vm.frames[i];
        auto &function = frame->function;
        size_t instruction = frame->ip - function->chunk.code.data() - 1;
        if (function->name == "error")
        {
            continue;
        }
        fprintf(stderr, "[line %d] in ",
                function->chunk.lines[instruction]);
        if (function->name == "")
        {
            std::string name = frame->name;
            if (name == "")
            {
                name = "script";
            }
            fprintf(stderr, "%s", (name + "\n").c_str());
        }
        else
        {
            fprintf(stderr, "%s()\n", function->name.c_str());
        }
    }
}