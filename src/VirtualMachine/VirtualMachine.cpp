#include "VirtualMachine.hpp"

void push(VM &vm, Value &value)
{
    vm.stack.push_back(value);
    vm.sp = &vm.stack.back();
}

Value pop(VM &vm)
{
    Value value = std::move(vm.stack.back());
    vm.stack.pop_back();
    vm.sp--;
    return value;
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

static void define_native(VM &vm, std::string name, NativeFunction function)
{
    Value native = native_val();
    native.get_native()->function = function;
    native.get_native()->name = name;
    vm.globals[name] = native;
}

static void define_global(VM &vm, std::string name, Value value)
{
    vm.globals[name] = value;
}

static EvaluateResult run(VM &vm)
{
#define READ_BYTE() (*frame->ip++)
#define READ_INT() (bytes_to_int(READ_BYTE(), READ_BYTE(), READ_BYTE(), READ_BYTE()))
#define READ_CONSTANT() (frame->function->chunk.constants[READ_INT()])

    // Define globals
    define_global(vm, "String", type_val("String"));
    define_global(vm, "Number", type_val("Number"));
    define_global(vm, "Boolean", type_val("Boolean"));
    define_global(vm, "List", type_val("List"));
    define_global(vm, "Object", type_val("Object"));
    define_global(vm, "Function", type_val("Function"));
    define_global(vm, "None", none_val());

    Value vm_ptr = pointer_val();
    vm_ptr.get_pointer()->value = &vm;
    define_global(vm, "__vm__", vm_ptr);

    // Define native functions
    define_native(vm, "print", print_builtin);
    define_native(vm, "println", println_builtin);
    define_native(vm, "clock", clock_builtin);
    define_native(vm, "string", to_string_builtin);
    define_native(vm, "number", to_number_builtin);
    define_native(vm, "insert", insert_builtin);
    define_native(vm, "append", append_builtin);
    define_native(vm, "remove", remove_builtin);
    define_native(vm, "remove_prop", remove_prop_builtin);
    define_native(vm, "dis", dis_builtin);
    define_native(vm, "length", length_builtin);
    define_native(vm, "info", info_builtin);
    define_native(vm, "type", type_builtin);
    define_native(vm, "copy", copy_builtin);
    define_native(vm, "sort", sort_builtin);
    define_native(vm, "__future__", future_builtin);
    define_native(vm, "__get_future__", get_future_builtin);
    define_native(vm, "__check_future__", check_future_builtin);
    define_native(vm, "exit", exit_builtin);
    define_native(vm, "__error__", error_builtin);
    define_native(vm, "load_lib", load_lib_builtin);

    CallFrame *frame = &vm.frames.back();
    frame->ip = frame->function->chunk.code.data();
    frame->frame_start = vm.stack.size();

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        printf("[ ");
        for (Value value : vm.stack)
        {
            printValue(value);
            printf(" ");
        }
        printf("]");
        printf("\n");
        disassemble_instruction(frame->function->chunk, (int)(size_t)(frame->ip - &frame->function->chunk.code[0]));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_EXIT:
        {
            for (auto &closure : vm.closed_values)
            {
                closure->closed = *closure->location;
                closure->location = &closure->closed;
            }
            return EVALUATE_OK;
        }
        case OP_RETURN:
        {
            if (frame->function->is_generator)
            {
                frame->function->generator_done = true;
            }
            Value return_value = pop(vm);
            return_value.meta.temp_non_const = false;
            if (frame->function->is_type_generator && return_value.is_object())
            {
                return_value.get_object()->type_name = frame->function->name;
            }
            int to_clean = vm.stack.size() - frame->sp;
            int instruction_index = frame->instruction_index;
            for (int i = 0; i < to_clean; i++)
            {
                Value &value = vm.stack.back();
                for (auto &closure : vm.closed_values)
                {
                    if (closure->location == &value)
                    {
                        closure->closed = value;
                        closure->location = &closure->closed;
                        break;
                    }
                }
                vm.stack.pop_back();
            }
            vm.frames.pop_back();
            frame = &vm.frames.back();
            frame->ip = &frame->function->chunk.code[instruction_index];
            push(vm, return_value);
            break;
        }
        case OP_YIELD:
        {
            frame->gen_stack.clear();
            Value return_value = pop(vm);
            int to_clean = vm.stack.size() - frame->sp;
            int instruction_index = frame->instruction_index;
            for (int i = 0; i < to_clean; i++)
            {
                Value &value = vm.stack.back();
                for (auto &closure : vm.closed_values)
                {
                    if (closure->location == &value)
                    {
                        closure->closed = value;
                        closure->location = &closure->closed;
                        break;
                    }
                }
                frame->gen_stack.insert(frame->gen_stack.begin(), value);
                vm.stack.pop_back();
            }
            *vm.gen_frames[frame->function->name] = *frame;
            vm.frames.pop_back();
            frame = &vm.frames.back();
            frame->ip = &frame->function->chunk.code[instruction_index];
            push(vm, return_value);
            break;
        }
        case OP_LOAD_THIS:
        {
            if (!frame->function->object)
            {
                Value none = none_val();
                push(vm, none);
            }
            else
            {
                Value &value = *frame->function->object;
                value.meta.temp_non_const = true;
                push(vm, value);
            }
            break;
        }
        case OP_LOAD_CONST:
        {
            Value constant = READ_CONSTANT();
            push(vm, constant);
            break;
        }
        case OP_LOAD:
        {
            int index = READ_INT();
            push(vm, vm.stack[index + frame->frame_start]);
            break;
        }
        case OP_SET:
        {
            int index = READ_INT();
            Value value = vm.stack[index + frame->frame_start];
            if (value.meta.is_const)
            {
                if (!value.meta.temp_non_const)
                {
                    runtimeError(vm, "Cannot modify const");
                    return EVALUATE_RUNTIME_ERROR;
                }
            }
            if (value.hooks.onChangeHook)
            {

                Value new_value = pop(vm);

                Value obj = object_val();
                obj.get_object()->keys = {"old", "current"};
                obj.get_object()->values["old"] = value;
                obj.get_object()->values["current"] = new_value;

                VM func_vm;
                std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
                main->name = "";
                main->arity = 0;
                main->chunk = Chunk();
                main->chunk.import_path = frame->function->chunk.import_path;
                CallFrame main_frame;
                main_frame.name = frame->name;
                main_frame.function = main;
                main_frame.sp = 0;
                main_frame.ip = main->chunk.code.data();
                main_frame.frame_start = 0;
                func_vm.frames.push_back(main_frame);

                add_constant(main->chunk, *value.hooks.onChangeHook);
                add_constant(main->chunk, obj);

                add_opcode(main->chunk, OP_LOAD_CONST, 1, 0);
                add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
                add_opcode(main->chunk, OP_CALL, 1, 0);
                add_code(main->chunk, OP_EXIT, 0);

                auto offsets = instruction_offsets(main_frame.function->chunk);
                main_frame.function->instruction_offsets = offsets;

                auto status = evaluate(func_vm);

                if (status != 0)
                {
                    exit(1);
                }

                obj.get_object()->values["current"].hooks.onChangeHook = value.hooks.onChangeHook;
                push(vm, obj.get_object()->values["current"]);
                vm.stack[index + frame->frame_start] = vm.stack.back();
                break;
            }
            vm.stack[index + frame->frame_start] = vm.stack.back();
            vm.stack[index + frame->frame_start].meta.is_const = false;
            break;
        }
        case OP_SET_FORCE:
        {
            int index = READ_INT();
            Value value = vm.stack[index + frame->frame_start];
            vm.stack[index + frame->frame_start] = vm.stack.back();
            break;
        }
        case OP_SET_PROPERTY:
        {
            Value value = pop(vm);
            Value accessor = pop(vm);
            Value container = pop(vm);

            if (container.meta.is_const)
            {
                if (!container.meta.temp_non_const)
                {
                    runtimeError(vm, "Cannot modify const");
                    return EVALUATE_RUNTIME_ERROR;
                }
            }
            if (container.is_object())
            {
                if (!accessor.is_string())
                {
                    runtimeError(vm, "Object accessor must be a string");
                    return EVALUATE_RUNTIME_ERROR;
                }
                Value current = container.get_object()->values[accessor.get_string()];

                if (current.hooks.onChangeHook)
                {
                    Value obj = object_val();
                    obj.get_object()->keys = {"old", "current"};
                    obj.get_object()->values["old"] = current;
                    obj.get_object()->values["current"] = value;

                    VM func_vm;
                    std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
                    main->name = "";
                    main->arity = 0;
                    main->chunk = Chunk();
                    main->chunk.import_path = frame->function->chunk.import_path;
                    CallFrame main_frame;
                    main_frame.name = frame->name;
                    main_frame.function = main;
                    main_frame.sp = 0;
                    main_frame.ip = main->chunk.code.data();
                    main_frame.frame_start = 0;
                    func_vm.frames.push_back(main_frame);

                    add_constant(main->chunk, *current.hooks.onChangeHook);
                    add_constant(main->chunk, obj);

                    add_opcode(main->chunk, OP_LOAD_CONST, 1, 0);
                    add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
                    add_opcode(main->chunk, OP_CALL, 1, 0);
                    add_code(main->chunk, OP_EXIT, 0);

                    auto offsets = instruction_offsets(main_frame.function->chunk);
                    main_frame.function->instruction_offsets = offsets;

                    auto status = evaluate(func_vm);

                    if (status != 0)
                    {
                        exit(1);
                    }

                    obj.get_object()->values["current"].hooks.onChangeHook = current.hooks.onChangeHook;
                    push(vm, obj.get_object()->values["current"]);
                    container.get_object()->values[accessor.get_string()] = obj.get_object()->values["current"];
                    break;
                }
                std::string &accessor_string = accessor.get_string();
                auto &keys = container.get_object()->keys;
                container.get_object()->values[accessor_string] = value;
                if (std::find(keys.begin(), keys.end(), accessor_string) == keys.end())
                {
                    keys.push_back(accessor_string);
                }
            }
            else if (container.is_list())
            {
                if (!accessor.is_number())
                {
                    runtimeError(vm, "List accessor must be a number");
                    return EVALUATE_RUNTIME_ERROR;
                }
                auto &list = *container.get_list();
                int acc = accessor.get_number();
                if (acc < 0)
                {
                    list.insert(list.begin(), value);
                }
                else if (acc >= list.size())
                {
                    list.push_back(value);
                }
                else
                {
                    list[acc] = value;
                }
            }
            else if (container.is_string())
            {
                if (!accessor.is_number())
                {
                    runtimeError(vm, "String accessor must be a number");
                    return EVALUATE_RUNTIME_ERROR;
                }
                if (!value.is_string())
                {
                    runtimeError(vm, "String values must be of type string");
                    return EVALUATE_RUNTIME_ERROR;
                }
                auto &string = container.get_string();
                int acc = accessor.get_number();
                // TODO: Fix this
                if (acc < 0)
                {
                    string = value.get_string() + string;
                }
                else if (acc >= string.length())
                {
                    string += value.get_string();
                }
                else
                {
                    string[acc] = value.get_string()[0];
                }
            }
            else
            {
                runtimeError(vm, "Object is not accessible");
                return EVALUATE_RUNTIME_ERROR;
            }
            push(vm, value);
            break;
        }
        case OP_LOAD_GLOBAL:
        {
            int flag = READ_INT();
            Value name = pop(vm);
            std::string &name_str = name.get_string();
            if (!vm.globals.count(name_str))
            {
                if (flag == 0)
                {
                    runtimeError(vm, "Global '" + name_str + "' is undefined");
                    return EVALUATE_RUNTIME_ERROR;
                }
                else if (flag == 1)
                {
                    Value none = none_val();
                    push(vm, none);
                    break;
                }
            }
            push(vm, vm.globals[name_str]);
            break;
        }
        case OP_MAKE_OBJECT:
        {
            int size = READ_INT();
            Value object = object_val();
            auto &object_obj = object.get_object();
            for (int i = 0; i < size; i++)
            {
                Value prop_value = pop(vm);
                Value prop_name = pop(vm);
                object_obj->keys.insert(object_obj->keys.begin(), prop_name.get_string());
                if (!prop_name.is_string())
                {
                    runtimeError(vm, "Object keys must evaluate to strings");
                    return EVALUATE_RUNTIME_ERROR;
                }
                object_obj->values[prop_name.get_string()] = prop_value;
            }
            push(vm, object);
            break;
        }
        case OP_MAKE_FUNCTION:
        {
            int count = READ_INT();
            Value function = pop(vm);
            for (int i = 0; i < count; i++)
            {
                function.get_function()->default_values.insert(function.get_function()->default_values.begin(), pop(vm));
            }
            push(vm, function);
            break;
        }
        case OP_MAKE_TYPE:
        {
            int size = READ_INT();
            Value type = type_val("");
            auto &type_obj = type.get_type();
            for (int i = 0; i < size; i++)
            {
                Value prop_type = pop(vm);
                Value prop_name = pop(vm);

                type_obj->types[prop_name.get_string()] = prop_type;
            }

            Value name = pop(vm);
            type_obj->name = name.get_string();
            push(vm, type);
            break;
        }
        case OP_MAKE_CONST:
        {
            vm.stack.back().meta.is_const = true;
            break;
        }
        case OP_MAKE_NON_CONST:
        {
            vm.stack.back().meta.is_const = false;
            break;
        }
        case OP_TYPE_DEFAULTS:
        {
            int size = READ_INT();
            auto &type = vm.stack[vm.stack.size() - (size * 2) - 1];
            for (int i = 0; i < size; i++)
            {
                Value prop_default = pop(vm);
                Value prop_name = pop(vm);

                type.get_type()->defaults[prop_name.get_string()] = prop_default;
            }
            break;
        }
        case OP_MAKE_TYPED:
        {
            Value type = pop(vm);
            Value &object = vm.stack.back();
            object.get_object()->type = type.get_type();
            break;
        }
        case OP_MAKE_CLOSURE:
        {
            int index = READ_INT();
            auto function = pop(vm).get_function();
            Value closure = function_val();
            auto closure_obj = closure.get_function();
            closure_obj->arity = function->arity;
            closure_obj->closed_var_indexes = function->closed_var_indexes;
            closure_obj->chunk = function->chunk;
            closure_obj->defaults = function->defaults;
            closure_obj->name = function->name;
            closure_obj->params = function->params;
            closure_obj->is_generator = function->is_generator;
            closure_obj->generator_init = function->generator_init;
            closure_obj->generator_done = function->generator_done;
            closure_obj->is_type_generator = function->is_type_generator;
            closure_obj->instruction_offsets = function->instruction_offsets;
            closure_obj->closed_vars = std::vector<std::shared_ptr<Closure>>();
            for (auto &var : closure_obj->closed_var_indexes)
            {
                int index = var.index;
                bool is_local = var.is_local;
                Value *value_pointer;
                if (is_local)
                {
                    value_pointer = &vm.stack[index + frame->frame_start];
                }
                else
                {
                    value_pointer = &*frame->function->closed_vars[index]->location;
                }
                Value &value = *value_pointer;
                auto hoisted = std::make_shared<Closure>();
                hoisted->location = &value;
                if (vm.closed_values.size() == 0)
                {
                    closure_obj->closed_vars.push_back(hoisted);
                    vm.closed_values.push_back(hoisted);
                }
                else
                {
                    bool found = false;
                    for (auto &cl : vm.closed_values)
                    {
                        if (cl->location == &value)
                        {
                            closure_obj->closed_vars.push_back(cl);
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        closure_obj->closed_vars.push_back(hoisted);
                        vm.closed_values.push_back(hoisted);
                    }
                }
            }
            function->closed_vars = closure_obj->closed_vars;
            push(vm, closure);
            break;
        }
        case OP_LOAD_CLOSURE:
        {
            int index = READ_INT();
            push(vm, *frame->function->closed_vars[index]->location);
            break;
        }
        case OP_SET_CLOSURE:
        {
            int index = READ_INT();
            Value value = *frame->function->closed_vars[index]->location;
            if (value.meta.is_const)
            {
                if (!value.meta.temp_non_const)
                {
                    runtimeError(vm, "Cannot modify const");
                    return EVALUATE_RUNTIME_ERROR;
                }
            }
            if (value.hooks.onChangeHook)
            {

                Value new_value = pop(vm);

                Value obj = object_val();
                obj.get_object()->keys = {"old", "current"};
                obj.get_object()->values["old"] = value;
                obj.get_object()->values["current"] = new_value;

                VM func_vm;
                std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
                main->name = "";
                main->arity = 0;
                main->chunk = Chunk();
                main->chunk.import_path = frame->function->chunk.import_path;
                CallFrame main_frame;
                main_frame.name = frame->name;
                main_frame.function = main;
                main_frame.sp = 0;
                main_frame.ip = main->chunk.code.data();
                main_frame.frame_start = 0;
                func_vm.frames.push_back(main_frame);

                add_constant(main->chunk, *value.hooks.onChangeHook);
                add_constant(main->chunk, obj);

                add_opcode(main->chunk, OP_LOAD_CONST, 1, 0);
                add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
                add_opcode(main->chunk, OP_CALL, 1, 0);
                add_code(main->chunk, OP_EXIT, 0);

                auto offsets = instruction_offsets(main_frame.function->chunk);
                main_frame.function->instruction_offsets = offsets;

                auto status = evaluate(func_vm);

                if (status != 0)
                {
                    exit(1);
                }

                obj.get_object()->values["current"].hooks.onChangeHook = value.hooks.onChangeHook;
                push(vm, obj.get_object()->values["current"]);
                *frame->function->closed_vars[index]->location = vm.stack.back();
                break;
            }
            *frame->function->closed_vars[index]->location = vm.stack.back();
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            int offset = READ_INT();
            if (is_falsey(vm.stack.back()))
            {
                frame->ip += offset;
            }
            break;
        }
        case OP_JUMP_IF_TRUE:
        {
            int offset = READ_INT();
            if (!is_falsey(vm.stack.back()))
            {
                frame->ip += offset;
            }
            break;
        }
        case OP_POP_JUMP_IF_FALSE:
        {
            int offset = READ_INT();
            if (is_falsey(vm.stack.back()))
            {
                frame->ip += offset;
            }
            pop(vm);
            break;
        }
        case OP_POP_JUMP_IF_TRUE:
        {
            int offset = READ_INT();
            if (!is_falsey(vm.stack.back()))
            {
                frame->ip += offset;
            }
            pop(vm);
            break;
        }
        case OP_JUMP:
        {
            int offset = READ_INT();
            frame->ip += offset;
            break;
        }
        case OP_JUMP_BACK:
        {
            int offset = READ_INT();
            frame->ip -= offset;
            break;
        }
        case OP_POP:
        {
            pop(vm);
            break;
        }
        case OP_LOOP:
        {
            int stack_size = vm.stack.size();
            uint8_t *bytes = int_to_bytes(stack_size);
            for (int i = 0; i < 4; i++)
            {
                frame->ip[i] = bytes[i];
            }
            READ_INT();
            break;
        }
        case OP_LOOP_END:
        {
            break;
        }
        case OP_ITER:
        {
            break;
        }
        case OP_BREAK:
        {
            int count = 1;
            int current_offset = (int)(size_t)(frame->ip - &frame->function->chunk.code[0]);
            int instruction_index = std::find(frame->function->instruction_offsets.begin(), frame->function->instruction_offsets.end(), current_offset) - frame->function->instruction_offsets.begin();
            int instruction = frame->function->instruction_offsets[instruction_index];
            int _instruction = frame->function->chunk.code[instruction];
            while (true)
            {
                instruction_index--;
                instruction = frame->function->instruction_offsets[instruction_index];
                int diff = frame->function->instruction_offsets[instruction_index + 1] - instruction;
                frame->ip -= diff;
                _instruction = frame->function->chunk.code[instruction];
                if (_instruction == OP_LOOP_END)
                {
                    count++;
                }
                else if (_instruction == OP_LOOP)
                {
                    count--;
                    if (count == 0)
                    {
                        break;
                    }
                }
            }
            READ_BYTE();
            int stack_size_start = READ_INT();
            int to_pop = vm.stack.size() - stack_size_start;

            for (int i = 0; i < to_pop; i++)
            {
                pop(vm);
            }
            // We need to go all the way down to OP_JUMP_BACK, but make sure to
            // skip any loops along the way

            count = 1;

            while (true)
            {

                instruction_index++;
                instruction = frame->function->instruction_offsets[instruction_index];
                _instruction = frame->function->chunk.code[instruction];
                int diff = frame->function->instruction_offsets[instruction_index + 1] - instruction;
                frame->ip += diff;

                if (_instruction == OP_LOOP)
                {
                    count++;
                }
                if (_instruction == OP_JUMP_BACK)
                {
                    count--;
                }

                if (count == 0)
                {
                    break;
                }
            }

            // while (_instruction != OP_JUMP_BACK)
            // {
            //     instruction_index++;
            //     instruction = frame->function->instruction_offsets[instruction_index];
            //     _instruction = frame->function->chunk.code[instruction];
            //     int diff = frame->function->instruction_offsets[instruction_index + 1] - instruction;
            //     frame->ip += diff;
            // }

            break;
        }
        case OP_CONTINUE:
        {
            int count = 1;
            int current_offset = (int)(size_t)(frame->ip - &frame->function->chunk.code[0]);
            int instruction_index = std::find(frame->function->instruction_offsets.begin(), frame->function->instruction_offsets.end(), current_offset) - frame->function->instruction_offsets.begin();
            int instruction = frame->function->instruction_offsets[instruction_index];
            int _instruction = frame->function->chunk.code[instruction];
            while (true)
            {
                instruction_index--;
                instruction = frame->function->instruction_offsets[instruction_index];
                int diff = frame->function->instruction_offsets[instruction_index + 1] - instruction;
                frame->ip -= diff;
                _instruction = frame->function->chunk.code[instruction];
                if (_instruction == OP_LOOP_END)
                {
                    count++;
                }
                else if (_instruction == OP_LOOP)
                {
                    count--;
                    if (count == 0)
                    {
                        break;
                    }
                }
            }

            READ_BYTE();
            int stack_size_start = READ_INT();
            int to_pop = vm.stack.size() - stack_size_start;

            for (int i = 0; i < to_pop; i++)
            {
                pop(vm);
            }

            // We need to go all the way down to OP_JUMP_BACK, but make sure to
            // skip any loops along the way

            count = 1;

            while (true)
            {
                instruction_index++;
                instruction = frame->function->instruction_offsets[instruction_index];
                _instruction = frame->function->chunk.code[instruction];
                int diff = frame->function->instruction_offsets[instruction_index + 1] - instruction;
                frame->ip += diff;

                if (_instruction == OP_LOOP)
                {
                    count++;
                }
                if (_instruction == OP_JUMP_BACK)
                {
                    count--;
                }

                if (_instruction == OP_ITER && count == 1)
                {
                    break;
                }

                if (count == 0)
                {
                    break;
                }
            }

            break;
        }
        case OP_BUILD_LIST:
        {
            Value list = list_val();
            int size = READ_INT();
            auto &list_val = list.get_list();
            for (int i = 0; i < size; i++)
            {
                list_val->insert(list_val->begin(), pop(vm));
            }
            push(vm, list);
            break;
        }
        case OP_ACCESSOR:
        {
            int flag = READ_INT();
            Value _index = pop(vm);
            Value _container = pop(vm);

            if (!_container.is_list() && !_container.is_object() && !_container.is_string())
            {
                if (flag == 1)
                {
                    push(vm, _container);
                    break;
                }
                runtimeError(vm, "Object is not accessable");
                return EVALUATE_RUNTIME_ERROR;
            }

            if (_container.is_list())
            {
                if (flag == 1)
                {
                    Value none = none_val();
                    push(vm, _container);
                    break;
                }
                if (!_index.is_number())
                {
                    runtimeError(vm, "Accessor must be a number");
                    return EVALUATE_RUNTIME_ERROR;
                }
                int index = _index.get_number();
                auto &list = _container.get_list();
                if (index >= list->size() || index < 0)
                {
                    Value none = none_val();
                    push(vm, none);
                }
                else
                {
                    push(vm, (*list)[index]);
                }
            }
            else if (_container.is_object())
            {
                if (!_index.is_string())
                {
                    runtimeError(vm, "Accessor must be a string");
                    return EVALUATE_RUNTIME_ERROR;
                }
                std::string &index = _index.get_string();
                auto &object = _container.get_object();
                if (!object->values.count(index))
                {
                    Value none = none_val();
                    push(vm, none);
                }
                else
                {
                    push(vm, object->values[index]);
                }
            }
            else if (_container.is_string())
            {
                if (flag == 1)
                {
                    Value none = none_val();
                    push(vm, _container);
                    break;
                }
                if (!_index.is_number())
                {
                    runtimeError(vm, "Accessor must be a number");
                    return EVALUATE_RUNTIME_ERROR;
                }
                int index = _index.get_number();
                auto &string = _container.get_string();
                if (index >= string.length() || index < 0)
                {
                    Value none = none_val();
                    push(vm, none);
                }
                else
                {
                    Value str = string_val(std::string(1, string[index]));
                    push(vm, str);
                }
            }

            if (flag == 1)
            {
                push(vm, _container);
            }
            break;
        }
        case OP_LEN:
        {
            Value list = pop(vm);
            if (!list.is_list())
            {
                runtimeError(vm, "Operand must be a list");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val(list.get_list()->size());
            push(vm, value);
            break;
        }
        case OP_UNPACK:
        {
            Value &list = vm.stack.back();
            if (!list.is_list())
            {
                runtimeError(vm, "Operand must be a list");
                return EVALUATE_RUNTIME_ERROR;
            }
            list.meta.unpack = true;
            break;
        }
        case OP_REMOVE_PUSH:
        {
            int index = READ_INT();
            int _index = vm.stack.size() - index;
            Value value = vm.stack[_index];
            vm.stack.erase(vm.stack.begin() + _index);
            push(vm, value);
            break;
        }
        case OP_SWAP_TOS:
        {
            Value v1 = pop(vm);
            Value v2 = pop(vm);
            push(vm, v1);
            push(vm, v2);
            break;
        }
        case OP_CALL:
        {
            int param_num = READ_INT();
            Value function = pop(vm);

            if (function.is_native())
            {
                auto &native_function = function.get_native();
                std::vector<Value> args;
                for (int i = 0; i < param_num; i++)
                {
                    Value arg = pop(vm);
                    if (arg.meta.unpack)
                    {
                        arg.meta.unpack = false;
                        for (auto &elem : *arg.get_list())
                        {
                            args.push_back(elem);
                        }
                    }
                    else
                    {
                        args.push_back(arg);
                    }
                }
                Value result = native_function->function(args);
                push(vm, result);
                break;
            }

            if (!function.is_function())
            {
                runtimeError(vm, "Object is not callable");
                return EVALUATE_RUNTIME_ERROR;
            }

            int status = call_function(vm, function, param_num, frame);

            if (status < 0)
            {
                return EVALUATE_RUNTIME_ERROR;
            }
            break;
        }
        case OP_CALL_METHOD:
        {
            int param_num = READ_INT();
            Value backup_function = pop(vm);
            Value object = pop(vm);
            Value function;

            if (!object.is_object())
            {
                function = backup_function;
                param_num++;
                push(vm, object);
            }
            else
            {
                function = pop(vm);
            }

            if (function.is_none())
            {
                function = backup_function;
                param_num++;
                push(vm, object);
            }

            if (function.is_native())
            {
                auto &native_function = function.get_native();
                std::vector<Value> args;
                for (int i = 0; i < param_num; i++)
                {
                    Value arg = pop(vm);
                    if (arg.meta.unpack)
                    {
                        arg.meta.unpack = false;
                        for (auto &elem : *arg.get_list())
                        {
                            args.push_back(elem);
                        }
                    }
                    else
                    {
                        args.push_back(arg);
                    }
                }
                Value result = native_function->function(args);
                push(vm, result);
                break;
            }

            if (!function.is_function())
            {
                runtimeError(vm, "Object is not callable");
                return EVALUATE_RUNTIME_ERROR;
            }

            int status = call_function(vm, function, param_num, frame, std::make_shared<Value>(object));

            if (status < 0)
            {
                return EVALUATE_RUNTIME_ERROR;
            }

            break;

            // auto& function_obj = function.get_function();
            // int positional_args = function_obj->arity - function_obj->defaults;

            // int num_captured = 0;
            // int num_unpacked = 0;
            // int capturing = -1;

            // for (int i = 0; i < param_num; i++) {
            //     Value arg = pop(vm);
            //     if (arg.meta.unpack) {
            //         arg.meta.unpack = false;
            //         num_unpacked--;
            //         for (int j = 0; j < arg.get_list()->size(); j++) {
            //             num_unpacked++;
            //             auto _arg = arg.get_list()->at(j);

            //             Value& constant = function_obj->chunk.constants[i+j];

            //             if (constant.is_list() && constant.meta.packer) {
            //                 capturing = i+j;
            //                 constant = copy(constant);
            //                 constant.get_list()->clear();
            //                 constant.get_list()->push_back(_arg);
            //             } else {
            //                 if (capturing >= 0) {
            //                     function_obj->chunk.constants[capturing].get_list()->push_back(_arg);
            //                     num_captured++;
            //                 } else {
            //                     function_obj->chunk.constants[i + j] = _arg;
            //                 }
            //             }
            //         }
            //     } else {
            //         Value& constant = function_obj->chunk.constants[i];
            //         if (constant.is_list() && constant.meta.packer) {
            //             capturing = i;
            //             constant = copy(constant);
            //             constant.get_list()->clear();
            //             constant.get_list()->push_back(arg);
            //         } else {
            //             if (capturing >= 0) {
            //                 function_obj->chunk.constants[capturing].get_list()->push_back(arg);
            //                 num_captured++;
            //             } else {
            //                 function_obj->chunk.constants[i] = arg;
            //             }
            //         }
            //     }
            // }

            // param_num += num_unpacked - num_captured;

            // if ((param_num < positional_args) || (param_num > function_obj->arity)) {
            //     runtimeError(vm, "Function '" + function_obj->name + "' expects " + std::to_string(function_obj->arity) + " argument(s)");
            //     return EVALUATE_RUNTIME_ERROR;
            // }

            // if (param_num < function_obj->arity) {
            //     // We have defaults we want to inject
            //     int default_index = 0;
            //     for (int i = param_num; i < function_obj->arity; i++) {
            //         Meta meta = function_obj->chunk.constants[i].meta;
            //         function_obj->chunk.constants[i] = function_obj->default_values[default_index];
            //         function_obj->chunk.constants[i].meta = meta;
            //         default_index++;
            //     }
            // }

            // //disassemble_chunk(function_obj->chunk, function_obj->name + "__");

            // CallFrame call_frame;
            // call_frame.frame_start = vm.stack.size();
            // call_frame.function = function_obj;
            // call_frame.function->object = std::make_shared<Value>(object);
            // call_frame.sp = vm.stack.size();
            // call_frame.ip = function_obj->chunk.code.data();

            // int instruction_index = frame->ip - &frame->function->chunk.code[0];

            // call_frame.instruction_index = instruction_index;

            // vm.frames.push_back(call_frame);
            // frame = &vm.frames.back();

            // break;
        }
        case OP_IMPORT:
        {
            int index = READ_INT();
            if (index == 0)
            {
                Value mod = pop(vm);
                Value path = pop(vm);

                if (mod.is_none())
                {
                    Lexer lexer(path.get_string());
                    lexer.tokenize();

                    Parser parser(lexer.nodes, lexer.file_name);
                    parser.parse(0, "_");
                    parser.remove_op_node(";");

                    auto current_path = std::filesystem::current_path();
                    auto parent_path = std::filesystem::path(path.get_string()).parent_path();
                    try
                    {
                        if (parent_path != "")
                        {
                            std::filesystem::current_path(parent_path);
                        }
                    }
                    catch (...)
                    {
                        runtimeError(vm, "No such file or directory: '" + parent_path.string() + "'");
                        return EVALUATE_RUNTIME_ERROR;
                    }

                    VM import_vm;
                    std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
                    main->name = "";
                    main->arity = 0;
                    main->chunk = Chunk();
                    main->chunk.import_path = frame->function->chunk.import_path;
                    CallFrame main_frame;
                    main_frame.name = frame->name;
                    main_frame.function = main;
                    main_frame.sp = 0;
                    main_frame.ip = main->chunk.code.data();
                    main_frame.frame_start = 0;
                    import_vm.frames.push_back(main_frame);

                    reset();
                    generate_bytecode(parser.nodes, main_frame.function->chunk);
                    add_code(main_frame.function->chunk, OP_EXIT);
                    auto offsets = instruction_offsets(main_frame.function->chunk);
                    main_frame.function->instruction_offsets = offsets;
                    evaluate(import_vm);

                    if (import_vm.status != 0)
                    {
                        exit(import_vm.status);
                    }

                    // Bring through globals in import
                    for (auto &global : import_vm.globals)
                    {
                        if (global.first != "__vm__")
                        {
                            vm.globals[global.first] = global.second;
                        }
                    }

                    std::filesystem::current_path(current_path);

                    for (int i = 0; i < import_vm.frames[0].function->chunk.public_variables.size(); i++)
                    {
                        auto &var = import_vm.frames[0].function->chunk.public_variables[i];
                        define_global(vm, var, import_vm.stack[i]);
                    }

                    break;
                }
                else
                {
                    // import mod : path
                    Lexer lexer(path.get_string());
                    lexer.tokenize();

                    Parser parser(lexer.nodes, lexer.file_name);
                    parser.parse(0, "_");
                    parser.remove_op_node(";");

                    auto current_path = std::filesystem::current_path();
                    auto parent_path = std::filesystem::path(path.get_string()).parent_path();
                    try
                    {
                        if (parent_path != "")
                        {
                            std::filesystem::current_path(parent_path);
                        }
                    }
                    catch (...)
                    {
                        runtimeError(vm, "No such file or directory: '" + parent_path.string() + "'");
                        return EVALUATE_RUNTIME_ERROR;
                    }

                    VM import_vm;
                    std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
                    main->name = "";
                    main->arity = 0;
                    main->chunk = Chunk();
                    main->chunk.import_path = frame->function->chunk.import_path;
                    CallFrame main_frame;
                    main_frame.name = frame->name;
                    main_frame.function = main;
                    main_frame.sp = 0;
                    main_frame.ip = main->chunk.code.data();
                    main_frame.frame_start = 0;
                    import_vm.frames.push_back(main_frame);

                    reset();
                    generate_bytecode(parser.nodes, main_frame.function->chunk);
                    add_code(main_frame.function->chunk, OP_EXIT);
                    auto offsets = instruction_offsets(main_frame.function->chunk);
                    main_frame.function->instruction_offsets = offsets;
                    evaluate(import_vm);

                    if (import_vm.status != 0)
                    {
                        exit(import_vm.status);
                    }

                    // Bring through globals in import
                    for (auto &global : import_vm.globals)
                    {
                        if (global.first != "__vm__")
                        {
                            vm.globals[global.first] = global.second;
                        }
                    }

                    std::filesystem::current_path(current_path);

                    Value import_obj = object_val();
                    auto &obj = import_obj.get_object();
                    for (int i = 0; i < import_vm.frames[0].function->chunk.public_variables.size(); i++)
                    {
                        auto &var = import_vm.frames[0].function->chunk.public_variables[i];
                        obj->values[var] = import_vm.stack[i];
                        obj->keys.push_back(var);
                    }

                    push(vm, import_obj);

                    break;
                }
            }
            else
            {
                // import [a, b, c] : path
                std::vector<std::string> names;

                for (int i = 0; i < index; i++)
                {
                    names.insert(names.begin(), pop(vm).get_string());
                }

                Value path = pop(vm);

                Lexer lexer(path.get_string());
                lexer.tokenize();

                Parser parser(lexer.nodes, lexer.file_name);
                parser.parse(0, "_");
                parser.remove_op_node(";");

                auto current_path = std::filesystem::current_path();
                auto parent_path = std::filesystem::path(path.get_string()).parent_path();
                try
                {
                    if (parent_path != "")
                    {
                        std::filesystem::current_path(parent_path);
                    }
                }
                catch (...)
                {
                    runtimeError(vm, "No such file or directory: '" + parent_path.string() + "'");
                    return EVALUATE_RUNTIME_ERROR;
                }

                VM import_vm;
                std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
                main->name = "";
                main->arity = 0;
                main->chunk = Chunk();
                main->chunk.import_path = frame->function->chunk.import_path;
                CallFrame main_frame;
                main_frame.name = frame->name;
                main_frame.function = main;
                main_frame.sp = 0;
                main_frame.ip = main->chunk.code.data();
                main_frame.frame_start = 0;
                import_vm.frames.push_back(main_frame);

                reset();
                generate_bytecode(parser.nodes, main_frame.function->chunk);
                add_code(main_frame.function->chunk, OP_EXIT);
                auto offsets = instruction_offsets(main_frame.function->chunk);
                main_frame.function->instruction_offsets = offsets;
                evaluate(import_vm);

                if (import_vm.status != 0)
                {
                    exit(import_vm.status);
                }

                // Bring through globals in import
                for (auto &global : import_vm.globals)
                {
                    if (global.first != "__vm__")
                    {
                        vm.globals[global.first] = global.second;
                    }
                }

                std::filesystem::current_path(current_path);

                for (auto &name : names)
                {
                    bool found = false;
                    for (int i = 0; i < import_vm.frames[0].function->chunk.public_variables.size(); i++)
                    {
                        if (name == import_vm.frames[0].function->chunk.public_variables[i])
                        {
                            push(vm, import_vm.stack[i]);
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        runtimeError(vm, "Cannot import variable '" + name + "' from '" + path.get_string() + "'");
                        return EVALUATE_RUNTIME_ERROR;
                    }
                }

                break;
            }
            break;
        }
        case OP_HOOK_ONCHANGE:
        {
            int index = READ_INT();
            Value function = pop(vm);
            vm.stack[index + frame->frame_start].hooks.onChangeHook = std::make_shared<Value>(function);
            break;
        }
        case OP_NEGATE:
        {
            Value constant = pop(vm);
            if (!constant.is_number())
            {
                runtimeError(vm, "Operand must be a number");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val(-constant.get_number());
            push(vm, value);
            break;
        }
        case OP_NOT:
        {
            Value constant = pop(vm);
            if (constant.is_none())
            {
                Value value = boolean_val(true);
                push(vm, value);
                break;
            }
            if (!constant.is_boolean())
            {
                runtimeError(vm, "Operand must be a boolean");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = boolean_val(!constant.get_boolean());
            push(vm, value);
            break;
        }
        case OP_ADD:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (v1.is_string() && v2.is_string())
            {
                Value value = string_val(v1.get_string() + v2.get_string());
                push(vm, value);
                break;
            }
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val(v1.get_number() + v2.get_number());
            push(vm, value);
            break;
        }
        case OP_SUBTRACT:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val(v1.get_number() - v2.get_number());
            push(vm, value);
            break;
        }
        case OP_MULTIPLY:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val(v1.get_number() * v2.get_number());
            push(vm, value);
            break;
        }
        case OP_DIVIDE:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val(v1.get_number() / v2.get_number());
            push(vm, value);
            break;
        }
        case OP_MOD:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val(fmod(v1.get_number(), v2.get_number()));
            push(vm, value);
            break;
        }
        case OP_POW:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val(pow(v1.get_number(), v2.get_number()));
            push(vm, value);
            break;
        }
        case OP_AND:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val((int)v1.get_number() & (int)v2.get_number());
            push(vm, value);
            break;
        }
        case OP_OR:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = number_val((int)v1.get_number() | (int)v2.get_number());
            push(vm, value);
            break;
        }
        case OP_EQ_EQ:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            Value value = boolean_val(is_equal(v1, v2));
            push(vm, value);
            break;
        }
        case OP_NOT_EQ:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            Value value = boolean_val(!is_equal(v1, v2));
            push(vm, value);
            break;
        }
        case OP_LT_EQ:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = boolean_val(v1.get_number() <= v2.get_number());
            push(vm, value);
            break;
        }
        case OP_GT_EQ:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = boolean_val(v1.get_number() >= v2.get_number());
            push(vm, value);
            break;
        }
        case OP_LT:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = boolean_val(v1.get_number() < v2.get_number());
            push(vm, value);
            break;
        }
        case OP_GT:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = boolean_val(v1.get_number() > v2.get_number());
            push(vm, value);
            break;
        }
        case OP_RANGE:
        {
            Value v2 = pop(vm);
            Value v1 = pop(vm);
            if (!v1.is_number() || !v2.is_number())
            {
                runtimeError(vm, "Operands must be numbers");
                return EVALUATE_RUNTIME_ERROR;
            }
            Value value = list_val();
            auto &list_value = value.get_list();
            for (int i = v1.get_number(); i < v2.get_number(); i++)
            {
                list_value->push_back(number_val(i));
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

EvaluateResult evaluate(VM &vm)
{
    return run(vm);
}

bool is_equal(Value &v1, Value &v2)
{
    if (v1.type != v2.type)
    {
        return false;
    }

    if (v1.is_number())
    {
        return v1.get_number() == v2.get_number();
    }
    if (v1.is_string())
    {
        return v1.get_string() == v2.get_string();
    }
    if (v1.is_boolean())
    {
        return v1.get_boolean() == v2.get_boolean();
    }
    if (v1.is_none())
    {
        return true;
    }

    return false;
}

bool is_falsey(Value &value)
{
    if (value.is_none() || value.is_boolean() && !value.get_boolean())
    {
        return true;
    }

    return false;
}

void freeVM(VM &vm)
{
    //
}

static int call_function(VM &vm, Value &function, int param_num, CallFrame *&frame, std::shared_ptr<Value> object)
{
    auto &function_obj = function.get_function();
    int positional_args = function_obj->arity - function_obj->defaults;

    if (!function_obj->is_generator || (function_obj->is_generator && !function_obj->generator_init))
    {
        int num_unpacked = 0;
        int num_captured = 0;
        int capturing = -1;

        for (int i = 0; i < param_num; i++)
        {
            Value arg = pop(vm);
            if (arg.meta.unpack)
            {
                arg.meta.unpack = false;
                num_unpacked--;
                for (int j = 0; j < arg.get_list()->size(); j++)
                {
                    num_unpacked++;
                    auto _arg = arg.get_list()->at(j);
                    Value &constant = function_obj->chunk.constants[i + j];
                    if (constant.is_list() && constant.meta.packer)
                    {
                        capturing = i + j;
                        constant = copy(constant);
                        constant.get_list()->clear();
                        constant.get_list()->push_back(_arg);
                    }
                    else
                    {
                        if (capturing >= 0)
                        {
                            function_obj->chunk.constants[capturing].get_list()->push_back(_arg);
                            num_captured++;
                        }
                        else
                        {
                            function_obj->chunk.constants[i + j] = _arg;
                        }
                    }
                }
            }
            else
            {
                Value &constant = function_obj->chunk.constants[i];
                if (constant.is_list() && constant.meta.packer)
                {
                    capturing = i;
                    constant = copy(constant);
                    constant.get_list()->clear();
                    constant.get_list()->push_back(arg);
                }
                else
                {
                    if (capturing >= 0)
                    {
                        function_obj->chunk.constants[capturing].get_list()->push_back(arg);
                        num_captured++;
                    }
                    else
                    {
                        function_obj->chunk.constants[i] = arg;
                    }
                }
            }
        }

        param_num += num_unpacked - num_captured;

        if ((param_num < positional_args) || (param_num > function_obj->arity))
        {
            runtimeError(vm, "Function '" + function_obj->name + "' expects " + std::to_string(function_obj->arity) + " argument(s)");
            return -1;
        }

        if (param_num < function_obj->arity)
        {
            // We have defaults we want to inject
            int default_index = param_num - positional_args;
            for (int i = param_num; i < function_obj->arity; i++)
            {
                Meta meta = function_obj->chunk.constants[i].meta;
                function_obj->chunk.constants[i] = function_obj->default_values[default_index];
                function_obj->chunk.constants[i].meta = meta;
                default_index++;
            }
        }
    }

    // disassemble_chunk(function_obj->chunk, function_obj->name + "__");

    if (function_obj->is_generator && !function_obj->generator_init)
    {
        auto function_copy = copy(function);
        auto &function_copy_obj = function_copy.get_function();
        function_copy_obj->generator_init = true;
        auto call_frame = std::make_shared<CallFrame>();
        call_frame->frame_start = vm.stack.size();
        call_frame->function = function_copy_obj;
        call_frame->sp = vm.stack.size();
        call_frame->ip = function_copy_obj->chunk.code.data();

        function_copy_obj->name = function_copy_obj->name + "_" + std::to_string(vm.coro_count++);

        int instruction_index = frame->ip - &frame->function->chunk.code[0];
        call_frame->instruction_index = instruction_index;
        vm.gen_frames[function_copy_obj->name] = call_frame;

        push(vm, function_copy);
        return 0;
    }
    else if (function_obj->is_generator && function_obj->generator_done)
    {
        Value none = none_val();
        push(vm, none);
        return 0;
    }
    else if (function_obj->is_generator && function_obj->generator_init)
    {
        auto &call_frame = vm.gen_frames[function_obj->name];
        call_frame->frame_start = vm.stack.size();
        call_frame->sp = vm.stack.size();

        if (param_num > 1)
        {
            runtimeError(vm, "Coroutine can only be called with one argument for parameter '_value'");
        }

        int _value_index = -1;

        if (param_num == 1)
        {
            for (int i = 0; i < call_frame->function->chunk.variables.size(); i++)
            {
                if (call_frame->function->chunk.variables[i] == "_value")
                {
                    _value_index = i;
                    break;
                }
            }

            if (_value_index == -1)
            {
                runtimeError(vm, "Missing variable '_value'");
            }

            call_frame->function->chunk.constants[_value_index] = pop(vm);
            call_frame->frame_start--;
            call_frame->sp--;
        }

        for (int i = 0; i < call_frame->gen_stack.size(); i++)
        {
            Value &value = call_frame->gen_stack[i];
            if (i == _value_index)
            {
                push(vm, call_frame->function->chunk.constants[_value_index]);
            }
            else
            {
                push(vm, value);
            }
        }

        int instruction_index = frame->ip - &frame->function->chunk.code[0];
        call_frame->instruction_index = instruction_index;

        vm.frames.push_back(*call_frame);
        frame = &*call_frame;
        return 0;
    }

    CallFrame call_frame;
    call_frame.frame_start = vm.stack.size();
    call_frame.function = function_obj;
    if (object)
    {
        call_frame.function->object = object;
    }
    call_frame.sp = vm.stack.size();
    call_frame.ip = function_obj->chunk.code.data();

    int instruction_index = frame->ip - &frame->function->chunk.code[0];
    call_frame.instruction_index = instruction_index;

    vm.frames.push_back(call_frame);
    frame = &vm.frames.back();

    return 0;
}

static Value print_builtin(std::vector<Value> &args)
{
    for (Value &arg : args)
    {
        printValue(arg);
    }
    return none_val();
}

static Value println_builtin(std::vector<Value> &args)
{
    for (Value &arg : args)
    {
        printValue(arg);
    }
    std::cout << '\n';
    return none_val();
}

static Value clock_builtin(std::vector<Value> &args)
{
    return number_val((double)clock() / CLOCKS_PER_SEC);
}

static Value dis_builtin(std::vector<Value> &args)
{
    if (args.size() != 1)
    {
        error("Function 'dis' expects 1 argument");
    }

    Value function = args[0];

    if (function.is_native())
    {
        std::ostringstream oss;
        oss << &function.get_native()->function;
        std::string address = oss.str();
        std::cout << "\n== C function ==\n<code> at " + address;
        return none_val();
    }

    if (!function.is_function())
    {
        error("Function 'dis' expects 1 'Function' argument");
    }

    std::cout << '\n';
    disassemble_chunk(function.get_function()->chunk, function.get_function()->name);

    return none_val();
}

static Value to_string_builtin(std::vector<Value> &args)
{
    if (args.size() != 1)
    {
        error("Function 'string' expects 1 argument");
    }

    Value value = args[0];

    return string_val(toString(value));
}

static Value to_number_builtin(std::vector<Value> &args)
{
    if (args.size() < 1 || args.size() > 2)
    {
        error("Function 'number' expects 1 or 2 argument");
    }

    Value value = args[0];

    switch (value.type)
    {
    case None:
        number_val(0);
    case Number:
        return value;
    case String:
    {
        std::string str = value.get_string();
        try
        {
            if (args.size() == 2)
            {
                int base = args[1].get_number();
                return number_val(std::stol(str, nullptr, base));
            }
            return number_val(std::stod(str));
        }
        catch (...)
        {
            // error("Cannot convert \"" + str + "\" to a number");
            return none_val();
        }
    };
    case Boolean:
    {
        if (value.get_boolean())
        {
            return number_val(1);
        }

        return number_val(0);
    };
    default:
        return number_val(0);
    }
}

static Value insert_builtin(std::vector<Value> &args)
{
    int arg_count = 3;
    if (args.size() != arg_count)
    {
        error("Function 'insert' expects " + std::to_string(arg_count) + " argument(s)");
    }

    Value list = args[0];
    Value value = args[1];
    Value pos = args[2];

    if (!list.is_list())
    {
        error("Function 'insert' expects argument 'list' to be a list");
    }

    if (!pos.is_number())
    {
        error("Function 'insert' expects argument 'pos' to be a number");
    }

    int pos_num = pos.get_number();
    auto &ls = list.get_list();

    if (pos_num < 0)
    {
        pos_num = 0;
    }
    else if (pos_num >= ls->size())
    {
        pos_num = ls->size();
    }

    ls->insert(ls->begin() + pos_num, value);

    return list;
}

static Value append_builtin(std::vector<Value> &args)
{
    int arg_count = 2;
    if (args.size() != arg_count)
    {
        error("Function 'append' expects " + std::to_string(arg_count) + " argument(s)");
    }

    Value list = args[0];
    Value value = args[1];

    if (!list.is_list())
    {
        error("Function 'append' expects argument 'list' to be a list");
    }

    auto &ls = list.get_list();

    ls->push_back(value);

    return list;
}

static Value remove_builtin(std::vector<Value> &args)
{
    int arg_count = 2;
    if (args.size() != arg_count)
    {
        error("Function 'remove' expects " + std::to_string(arg_count) + " argument(s)");
    }

    Value list = args[0];
    Value pos = args[1];

    if (!list.is_list())
    {
        error("Function 'remove' expects argument 'list' to be a list");
    }

    if (!pos.is_number())
    {
        error("Function 'remove' expects argument 'pos' to be a number");
    }

    int pos_num = pos.get_number();
    auto &ls = list.get_list();

    if (pos_num < 0 || pos_num >= ls->size())
    {
        return list;
    }

    if (ls->size() > 0)
    {
        ls->erase(ls->begin() + pos_num);
    }

    return list;
}

static Value remove_prop_builtin(std::vector<Value> &args)
{
    int arg_count = 2;
    if (args.size() != arg_count)
    {
        error("Function 'remove_prop' expects " + std::to_string(arg_count) + " argument(s)");
    }

    Value obj = args[0];
    Value name = args[1];

    if (!obj.is_object())
    {
        error("Function 'remove_prop' expects argument 'object' to be an object");
    }

    if (!name.is_string())
    {
        error("Function 'remove_prop' expects argument 'name' to be a string");
    }

    std::string &_name = name.get_string();
    auto &_obj = obj.get_object();

    _obj->values.erase(_name);
    _obj->keys.erase(std::remove(_obj->keys.begin(), _obj->keys.end(), _name), _obj->keys.end());

    return obj;
}

static Value length_builtin(std::vector<Value> &args)
{
    if (args.size() != 1)
    {
        error("Function 'length' expects 1 argument");
    }

    Value value = args[0];

    switch (value.type)
    {
    case List:
    {
        return number_val(value.get_list()->size());
    }
    case Object:
    {
        return number_val(value.get_object()->values.size());
    }
    case Type:
    {
        return number_val(value.get_type()->types.size());
    }
    case String:
    {
        return number_val(value.get_string().length());
    }
    default:
    {
        return none_val();
    }
    }
}

static Value type_builtin(std::vector<Value> &args)
{
    if (args.size() != 1)
    {
        error("Function 'type' expects 1 argument");
    }

    Value value = args[0];

    switch (value.type)
    {
    case Number:
    {
        return string_val("Number");
    }
    case String:
    {
        return string_val("String");
    }
    case Boolean:
    {
        return string_val("Boolean");
    }
    case Function:
    {
        return string_val("Function");
    }
    case List:
    {
        return string_val("List");
    }
    case Object:
    {
        return string_val("Object");
    }
    case Pointer:
    {
        return string_val("Pointer");
    }
    case Native:
    {
        return string_val("Native");
    }
    case Type:
    {
        return string_val("Type");
    }
    case None:
    {
        return string_val("None");
    }
    }
}

static Value info_builtin(std::vector<Value> &args)
{
    if (args.size() != 1)
    {
        error("Function 'info' expects 1 argument");
    }

    Value value = args[0];

    Value info = object_val();
    auto &obj = info.get_object();

    switch (value.type)
    {
    case Function:
    {
        obj->keys = {"name", "arity", "params", "generator", "init", "done"};
        auto &func = value.get_function();
        obj->values["name"] = string_val(func->name);
        obj->values["arity"] = number_val(func->arity);
        obj->values["params"] = list_val();
        for (auto &param : func->params)
        {
            obj->values["params"].get_list()->push_back(string_val(param));
        }
        obj->values["generator"] = boolean_val(func->is_generator);
        obj->values["init"] = boolean_val(func->generator_init);
        obj->values["done"] = boolean_val(func->generator_done);
        return info;
    }
    case Native:
    {
        obj->keys = {"name"};
        auto &func = value.get_native();
        obj->values["name"] = string_val(func->name);
        return info;
    }
    case Object:
    {
        obj->keys = {"type", "typename", "keys", "values"};
        auto &object = value.get_object();
        obj->values["type"] = object->type == nullptr ? none_val() : string_val(object->type->name);
        obj->values["typename"] = string_val(object->type_name);
        obj->values["keys"] = list_val();
        obj->values["values"] = list_val();
        for (std::string &key : object->keys)
        {
            obj->values["keys"].get_list()->push_back(string_val(key));
        }
        for (auto &prop : object->values)
        {
            std::string key = prop.first;
            if (std::find(object->keys.begin(), object->keys.end(), key) == object->keys.end())
            {
                obj->values["keys"].get_list()->insert(obj->values["keys"].get_list()->begin(), string_val(key));
            }
            obj->values["values"].get_list()->push_back(prop.second);
        }
        return info;
    }
    default:
    {
        return info;
    }
    }
}

static Value load_lib_builtin(std::vector<Value> &args)
{
    int arg_count = 2;
    if (args.size() != arg_count)
    {
        error("Function 'load_lib' expects " + std::to_string(arg_count) + " argument(s)");
    }

    Value path = args[0];
    Value func_list = args[1];

    if (!path.is_string())
    {
        error("Function 'load_lib' expects arg 'path' to be a string");
    }

    if (!func_list.is_list())
    {
        error("Function 'load_lib' expects arg 'func_list' to be a list");
    }

    Value lib_obj = object_val();

#if __APPLE__ || __linux__

    void *handle = dlopen(path.get_string().c_str(), RTLD_LAZY);

    if (!handle)
    {
        error("Cannot open library: " + std::string(dlerror()));
    }

    typedef void (*load_t)(VM &vm);

    auto &obj = lib_obj.get_object();

    for (auto &name : *func_list.get_list())
    {
        if (!name.is_string())
        {
            error("Function names must be strings");
        }

        NativeFunction fn = (NativeFunction)dlsym(handle, name.get_string().c_str());
        Value native = native_val();
        native.get_native()->function = fn;
        obj->values[name.get_string()] = native;
        obj->keys.push_back(name.get_string());
    }

#else

    typedef void(__cdecl * load_t)(VM & vm);
    HINSTANCE hinstLib;
    load_t loadFuncAddress;

    std::string path_str = path.get_string() + ".exe";

    hinstLib = LoadLibrary(TEXT(path_str.c_str()));

    if (hinstLib == NULL)
    {
        error("Cannot open library: " + path_str);
    }

    auto &obj = lib_obj.get_object();

    for (auto &name : *func_list.get_list())
    {
        if (!name.is_string())
        {
            error("Function names must be strings");
        }

        NativeFunction fn = (NativeFunction)GetProcAddress(hinstLib, name.get_string().c_str());
        Value native = native_val();
        native.get_native()->function = fn;
        obj->values[name.get_string()] = native;
        obj->keys.push_back(name.get_string());
    }

#endif

    return lib_obj;
}

Value copy(Value &value)
{
    switch (value.type)
    {
    case Function:
    {
        Value new_func = function_val();
        new_func.get_function()->arity = value.get_function()->arity;
        new_func.get_function()->chunk = value.get_function()->chunk;
        new_func.get_function()->closed_var_indexes = value.get_function()->closed_var_indexes;
        new_func.get_function()->closed_vars = value.get_function()->closed_vars;
        new_func.get_function()->default_values = value.get_function()->default_values;
        new_func.get_function()->generator_done = value.get_function()->generator_done;
        new_func.get_function()->generator_init = value.get_function()->generator_init;
        new_func.get_function()->is_generator = value.get_function()->is_generator;
        new_func.get_function()->defaults = value.get_function()->defaults;
        new_func.get_function()->is_type_generator = value.get_function()->is_type_generator;
        new_func.get_function()->name = value.get_function()->name;
        new_func.get_function()->object = value.get_function()->object;
        return new_func;
    }
    case List:
    {
        Value new_list = list_val();
        new_list.meta = value.meta;
        for (auto elem : *value.get_list())
        {
            new_list.get_list()->push_back(copy(elem));
        }
        return new_list;
    }
    case Object:
    {
        Value new_object = object_val();
        new_object.get_object()->type = value.get_object()->type;
        new_object.get_object()->type_name = value.get_object()->type_name;
        for (std::string key : value.get_object()->keys)
        {
            new_object.get_object()->keys.push_back(key);
        }
        for (auto prop : value.get_object()->values)
        {
            new_object.get_object()->values[prop.first] = copy(prop.second);
        }
        return new_object;
    }
    default:
    {
        return value;
    }
    }
}

static Value copy_builtin(std::vector<Value> &args)
{
    if (args.size() != 1)
    {
        error("Function 'copy' expects 1 argument");
    }

    Value value = args[0];
    return copy(value);
}

static Value sort_builtin(std::vector<Value> &args)
{
    if (args.size() != 2)
    {
        error("Function 'sort' expects 2 argument");
    }

    Value value = args[0];
    Value function = args[1];

    if (!value.is_list() || !function.is_function())
    {
        error("Function 'sort' expects arg 'list' to be a list and arg 'function' to be a function");
    }

    if (value.get_list()->size() < 2)
    {
        return value;
    }

    auto &func = function.get_function();

    Value new_list = copy(value);

    VM func_vm;
    std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
    main->name = "";
    main->arity = 0;
    main->chunk = Chunk();
    CallFrame main_frame;
    main_frame.function = main;
    main_frame.sp = 0;
    main_frame.ip = main->chunk.code.data();
    main_frame.frame_start = 0;
    func_vm.frames.push_back(main_frame);

    add_constant(main->chunk, function);
    add_constant(main->chunk, new_list.get_list()->at(1));
    add_constant(main->chunk, new_list.get_list()->at(0));

    add_opcode(main->chunk, OP_LOAD_CONST, 1, 0);
    add_opcode(main->chunk, OP_LOAD_CONST, 2, 0);
    add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
    add_opcode(main->chunk, OP_CALL, 2, 0);
    add_code(main->chunk, OP_EXIT, 0);

    auto offsets = instruction_offsets(main_frame.function->chunk);
    main_frame.function->instruction_offsets = offsets;

    std::sort(new_list.get_list()->begin(), new_list.get_list()->end(),
              [&func_vm, &function](const Value &lhs, const Value &rhs)
              {
                  auto &frame = func_vm.frames.back();
                  frame.function->chunk.constants[1] = rhs;
                  frame.function->chunk.constants[2] = lhs;

                  evaluate(func_vm);

                  if (func_vm.status != 0)
                  {
                      error("Error in sort function");
                  }

                  if (!func_vm.stack.back().is_boolean())
                  {
                      return false;
                  }

                  return func_vm.stack.back().get_boolean();
              });

    return new_list;
}

static Value exit_builtin(std::vector<Value> &args)
{
    if (args.size() != 1)
    {
        error("Function 'exit' expects 1 argument");
    }

    Value value = args[0];

    if (!value.is_number())
    {
        error("Function 'exit' expects 1 number argument");
    }

    exit(value.get_number());

    return value;
}

static Value error_builtin(std::vector<Value> &args)
{
    if (args.size() != 2)
    {
        error("Function 'error' expects 1 argument");
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

static Value future_builtin(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function '__future__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value func = args[0];
    Value vm = args[1];

    if (!func.is_function())
    {
        error("Function '__future__' expects argument 'function' to be a Function");
    }

    if (!vm.is_pointer())
    {
        error("Function '__future__' expects argument 'vm' to be a Pointer");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function '__future__' expects argument 'function' to be a Function with 0 parameters");
    }

    VM *_vm = (VM *)(vm.get_pointer()->value);

    auto _future = std::async(std::launch::async, [vm = std::move(_vm), func = std::move(func)]() mutable
                              {
        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        add_constant(main->chunk, func);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 0, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);

        return func_vm.stack.back(); })
                       .share();

    auto f = new std::shared_future<Value>(_future);

    auto future_ref = pointer_val();
    auto future = (void *)(f);
    future_ref.get_pointer()->value = future;
    return future_ref;
}

static Value get_future_builtin(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function '__get_future__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value future_ptr = args[0];

    if (!future_ptr.is_pointer())
    {
        error("Function '__get_future__' expects argument 'function' to be a Pointer");
    }

    auto future = (std::shared_future<Value> *)(future_ptr.get_pointer()->value);

    if (future->valid())
    {
        Value value = future->get();
        delete future;
        return value;
    }

    return none_val();
}

static Value check_future_builtin(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function '__check_future__' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value future_ptr = args[0];

    if (!future_ptr.is_pointer())
    {
        error("Function '__check_future__' expects argument 'function' to be a Pointer");
    }

    auto future = (std::shared_future<Value> *)(future_ptr.get_pointer()->value);

    if (future->valid())
    {
        auto is_ready = future->wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        return boolean_val(is_ready);
    }

    return boolean_val(false);
}