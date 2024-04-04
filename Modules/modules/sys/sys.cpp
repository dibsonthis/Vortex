#include <iostream>
#include "include/Vortex.hpp"

extern "C" Value __error__(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        return error_object("Function '__error__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value message = args[0];
    Value vm = args[1];

    if (!message.is_string())
    {
        return error_object("Function 'error' expects argument 'message' to be a string");
    }

    if (!vm.is_pointer())
    {
        return error_object("Function 'error' expects argument 'vm' to be a pointer");
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
        return error_object("Function '__stack__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value vm = args[0];

    if (!vm.is_pointer())
    {
        return error_object("Function '__stack__' expects argument 'vm' to be a pointer");
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
        return error_object("Function '__globals__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value vm = args[0];

    if (!vm.is_pointer())
    {
        return error_object("Function '__globals__' expects argument 'vm' to be a pointer");
    }

    VM *_vm = (VM *)(vm.get_pointer()->value);

    Value object = object_val();

    for (auto value : _vm->globals)
    {
        object.get_object()->values[value.first] = value.second;
        object.get_object()->keys.push_back(value.first);
    }

    return object;
}

extern "C" Value __frame__(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        return error_object("Function '__frame__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value vm = args[0];
    Value depth = args[1];

    if (!vm.is_pointer())
    {
        return error_object("Function '__frame__' expects argument 'vm' to be a pointer");
    }

    if (!depth.is_number())
    {
        return error_object("Function '__frame__' expects argument 'depth' to be a number");
    }

    VM *_vm = (VM *)(vm.get_pointer()->value);
    int _depth = depth.get_number();

    if (_depth < 1)
    {
        _depth = 1;
    }

    if (_depth > _vm->frames.size())
    {
        _depth = _vm->frames.size();
    }

    CallFrame frame = _vm->frames[_vm->frames.size() - _depth];

    size_t instr = frame.ip - frame.function->chunk.code.data() - 1;

    Value obj = object_val();

    obj.get_object()->values["name"] = string_val(frame.function->name);
    obj.get_object()->values["level"] = number_val(_vm->frames.size() - _depth);
    obj.get_object()->values["line"] = number_val(frame.function->chunk.lines[instr]);
    obj.get_object()->values["path"] = string_val(frame.function->name == "" ? frame.name : frame.function->import_path);
    obj.get_object()->values["id"] = number_val(reinterpret_cast<intptr_t>(frame.function.get()));
    obj.get_object()->keys = {"name", "level", "line", "path", "id"};
    return obj;
}

extern "C" Value __system__(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        return error_object("Function '__system__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value command = args[0];
    Value vm = args[1];

    if (!command.is_string())
    {
        return error_object("Function '__system__' expects argument 'command' to be a string");
    }

    if (!vm.is_pointer())
    {
        return error_object("Function '__system__' expects argument 'vm' to be a pointer");
    }

    system(command.get_string().c_str());

    return none_val();
}

extern "C" Value __argc__(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        return error_object("Function '__argc__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value vm = args[0];

    if (!vm.is_pointer())
    {
        return error_object("Function '__argc__' expects argument 'vm' to be a pointer");
    }

    VM *_vm = (VM *)(vm.get_pointer()->value);

    return number_val(_vm->argc);
}

extern "C" Value __argv__(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        return error_object("Function '__argv__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value vm = args[0];

    if (!vm.is_pointer())
    {
        return error_object("Function '__argv__' expects argument 'vm' to be a pointer");
    }

    VM *_vm = (VM *)(vm.get_pointer()->value);

    Value argv_obj = object_val();

    for (int i = 0; i < _vm->argc; i++)
    {
        std::string s(_vm->argv[i]);
        std::string key = std::to_string(i);
        argv_obj.get_object()->keys.push_back(key);
        argv_obj.get_object()->values[key] = string_val(s);
    }

    return argv_obj;
}