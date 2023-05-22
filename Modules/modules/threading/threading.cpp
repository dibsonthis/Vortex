#include <thread>
#include <future>
#include <mutex>
#include "../../Vortex.hpp"
#include "../../../src/utils/utils.cpp"
#include "../../../src/Lexer/Lexer.cpp"
#include "../../../src/Parser/Parser.cpp"
#include "../../../src/Interpreter/Interpreter.cpp"

/* Define Vars */

Interpreter _interpreter = Interpreter();
std::reference_wrapper<Interpreter> _interpreter_ref = std::ref(_interpreter);

VortexObj thread(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 2 arguments");
    }

    VortexObj func = args[0];
    VortexObj func_args = args[1];

    if (func->type != NodeType::FUNC || func_args->type != NodeType::LIST) {
        error_and_exit("Function '" + name + "' expects 1 Function argument and one List argument");
    }

    VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
    func_call->_Node.FunctionCall().name = func->_Node.Function().name;
    func_call->_Node.FunctionCall().args = func_args->_Node.List().elements;

    auto future = std::async(std::launch::async, [func = std::move(func), func_call = std::move(func_call)] () mutable {
        auto interp = Interpreter();
        interp.global_symbol_table = _interpreter_ref.get().global_symbol_table;
        interp.current_symbol_table = _interpreter_ref.get().current_symbol_table;
        node_ptr res = interp.eval_func_call(func_call, func);
        return res;
    }).share();

    uint32_t id = _interpreter_ref.get()._futures.size();
    _interpreter_ref.get()._futures[id] = future;

    VortexObj future_ref = new_number_node(id);
    future_ref->_Node.Number().value = id;

    return future_ref;
}

VortexObj get(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    if (args[0]->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 1 Number argument");
    }

    auto id = args[0]->_Node.Number().value;

    if (!_interpreter_ref.get()._futures.count(id)) {
        error_and_exit(std::to_string(id) + " is not a valid future ID");
    }

    auto f = _interpreter_ref.get()._futures[args[0]->_Node.Number().value];

    VortexObj res = f.get();
    _interpreter_ref.get()._futures.erase(args[0]->_Node.Number().value);
    return res;
}

VortexObj ready(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    if (args[0]->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 1 Number argument");
    }

    auto id = args[0]->_Node.Number().value;

    if (!_interpreter_ref.get()._futures.count(id)) {
        error_and_exit(std::to_string(id) + " is not a valid future ID");
    }

    auto f = _interpreter_ref.get()._futures[args[0]->_Node.Number().value];

    VortexObj is_ready_node = new_vortex_obj(NodeType::BOOLEAN);
    bool ready = f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    is_ready_node->_Node.Boolean().value = ready;
    return is_ready_node;
}

/* Implement load [Optional] */

extern "C" void load(Interpreter& interpreter) {
   _interpreter_ref = std::ref(interpreter);
   _interpreter = _interpreter_ref.get();
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "thread") {
        return thread(name, args);
    }
    if (name == "get") {
        return get(name, args);
    }
    if (name == "ready") {
        return ready(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}