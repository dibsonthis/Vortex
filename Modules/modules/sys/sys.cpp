#include <iostream>
#include "include/Vortex.hpp"

extern "C" Value __error__(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function '__error__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value message = args[0];
    Value vm = args[1];

    if (!message.is_string())
    {
        error("Function 'exit' expects argument 'message' to be a string");
    }

    if (!vm.is_pointer())
    {
        error("Function 'exit' expects argument 'vm' to be a pointer");
    }

    VM *_vm = (VM *)(vm.get_pointer()->value);
    runtimeError(*_vm, message.get_string());
    exit(0);

    return message;
}

extern "C" Value __stack__(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function '__stack__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value vm = args[0];

    if (!vm.is_pointer())
    {
        error("Function '__stack__' expects argument 'vm' to be a pointer");
    }

    VM *_vm = (VM *)(vm.get_pointer()->value);

    Value list = list_val();

    for (Value value : _vm->stack)
    {
        list.get_list()->push_back(value);
    }

    return list;
}

extern "C" Value __globals__(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function '__globals__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value vm = args[0];

    if (!vm.is_pointer())
    {
        error("Function '__globals__' expects argument 'vm' to be a pointer");
    }

    VM *_vm = (VM *)(vm.get_pointer()->value);

    Value object = object_val();

    for (auto value : _vm->globals)
    {
        object.get_object()->values[value.first] = value.second;
    }

    return object;
}

extern "C" Value __frame__(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function '__stack__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value vm = args[0];

    if (!vm.is_pointer())
    {
        error("Function '__stack__' expects argument 'vm' to be a pointer");
    }

    VM *_vm = (VM *)(vm.get_pointer()->value);

    Value obj = object_val();

    CallFrame frame = _vm->frames[_vm->frames.size() - 2];
    obj.get_object()->values["name"] = string_val(frame.function->name);
    obj.get_object()->values["level"] = number_val(_vm->frames.size() - 1);

    return obj;
}

extern "C" Value __system__(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function '__system__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value command = args[0];
    Value vm = args[1];

    if (!command.is_string())
    {
        error("Function 'system' expects argument 'command' to be a string");
    }

    if (!vm.is_pointer())
    {
        error("Function 'system' expects argument 'vm' to be a pointer");
    }

    system(command.get_string().c_str());

    return none_val();
}