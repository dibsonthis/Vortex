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
std::vector<std::shared_future<VortexObj>> futures;
std::vector<std::shared_ptr<std::mutex>> mutexes;
// std::mutex lock;
//std::shared_future<VortexObj> future;

/* Implement Lib Functions */

// VortexObj async(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 2) {
//         error_and_exit("Function '" + name + "' expects 2 arguments");
//     }

//     VortexObj func = args[0];
//     VortexObj func_args = args[1];

//     if (func->type != NodeType::FUNC || func_args->type != NodeType::LIST) {
//         error_and_exit("Function '" + name + "' expects 1 Function argument and one List argument");
//     }

//     VortexObj func_call = new_vortex_obj(NodeType::FUNC_CALL);
//     func_call->_Node.FunctionCall().name = func->_Node.Function().name;
//     func_call->_Node.FunctionCall().args = func_args->_Node.List().elements;

//     auto lock = std::make_shared<std::mutex>();
//     mutexes.push_back(std::move(lock));

//     // This will be the write-end
//     std::promise<VortexObj> promise;

//     // This will be the read-end
//     auto future = promise.get_future().share();
//     futures.push_back(std::move(future));
//     // future = promise.get_future().share();

//     // Launch f asynchronously
//     std::thread thread([promise = std::move(promise), func = std::move(func), func_call = std::move(func_call)] () mutable {
//         //std::lock_guard<std::mutex> guard(lock);
//         auto lock = mutexes.back();
//         std::lock_guard<std::mutex> guard(*lock);
//         auto interp = Interpreter();
//         //interp.global_symbol_table = _interpreter_ref.get().global_symbol_table;
//         node_ptr res = interp.eval_func_call(func_call, func);
//         promise.set_value(res);
//     });

//     VortexObj future_ptr = new_vortex_obj(NodeType::POINTER);
//     future_ptr->_Node.Pointer().value = &futures.back();

//     thread.detach();

//     return future_ptr;
// }

// VortexObj await(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 1) {
//         error_and_exit("Function '" + name + "' expects 1 argument");
//     }

//     if (args[0]->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects 1 Pointer argument");
//     }

//     auto f = (std::shared_future<VortexObj>*)args[0]->_Node.Pointer().value;

//     VortexObj res = f->get();
//     return res;
// }

// VortexObj is_ready(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 1) {
//         error_and_exit("Function '" + name + "' expects 1 argument");
//     }

//     if (args[0]->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects 1 Pointer argument");
//     }

//     auto f = (std::shared_future<VortexObj>*)args[0]->_Node.Pointer().value;
//     VortexObj is_ready_node = new_vortex_obj(NodeType::BOOLEAN);
//     bool ready = f->wait_for(std::chrono::seconds(0)) == std::future_status::ready;
//     is_ready_node->_Node.Boolean().value = ready;
//     return is_ready_node;
// }

VortexObj async(std::string name, std::vector<VortexObj> args) {

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

    // This will be the read-end
    auto future = std::async(std::launch::async, [func = std::move(func), func_call = std::move(func_call)] () mutable {
        auto interp = Interpreter();
        interp.global_symbol_table = _interpreter_ref.get().global_symbol_table;
        node_ptr res = interp.eval_func_call(func_call, func);
        return res;
    }).share();

    //_interpreter_ref.get()._futures.push_back(future);
    auto future_addr = &future;
    _interpreter_ref.get()._futures[future_addr] = future;

    VortexObj future_ptr = new_vortex_obj(NodeType::POINTER);
    future_ptr->_Node.Pointer().value = future_addr;

    return future_ptr;
}

VortexObj await(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    if (args[0]->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects 1 Pointer argument");
    }

    // auto f = (std::shared_future<VortexObj>*)args[0]->_Node.Pointer().value;
    auto f = _interpreter_ref.get()._futures[args[0]->_Node.Pointer().value];

    VortexObj res = f.get();
    //_interpreter_ref.get()._futures.erase(args[0]->_Node.Pointer().value);
    return res;
}

VortexObj is_ready(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    if (args[0]->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects 1 Pointer argument");
    }

    // auto f = (std::shared_future<VortexObj>*)args[0]->_Node.Pointer().value;
    // auto f = &_interpreter_ref.get()._futures.back();
    auto f = _interpreter_ref.get()._futures[args[0]->_Node.Pointer().value];
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
    if (name == "async") {
        return async(name, args);
    }
    if (name == "await") {
        return await(name, args);
    }
    if (name == "is_ready") {
        return is_ready(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}