#pragma once
#include "../Interpreter.hpp"

node_ptr Interpreter::eval_load_lib(node_ptr& node) {
    auto args = node->_Node.FunctionCall().args;
    if (args.size() != 1) {
        return throw_error("Library loading expects 1 argument");
    }

    node_ptr path = eval_node(args[0]);

    if (path->type != NodeType::STRING) {
        return throw_error("Library loading expects 1 string argument");
    }

    node_ptr lib_node = new_node(NodeType::LIB);

    #if GCC_COMPILER
        #if __apple__ || __linux__
            void* handle = dlopen(path->_Node.String().value.c_str(), RTLD_LAZY);
            
            if (!handle) {
                return throw_error("Cannot open library: " + std::string(dlerror()));
            }

            typedef node_ptr (*call_function_t)(std::string name, std::vector<node_ptr> args);
            typedef void (*load_t)(Interpreter*& interpreter);

            dlerror();

            call_function_t call_function = (call_function_t) dlsym(handle, "call_function");
            const char* dlsym_error_call_function = dlerror();

            if (dlsym_error_call_function) {
                dlclose(handle);
                return throw_error("Error loading symbol 'call_function': " + std::string(dlsym_error_call_function));
            }

            load_t load = (load_t) dlsym(handle, "load");
            const char* dlsym_error_load = dlerror();

            if (!dlsym_error_load) {
                load(this);
            }

            lib_node->_Node.Lib().handle = handle;
            lib_node->_Node.Lib().call_function = call_function;

        #else

            typedef node_ptr (__cdecl *call_function_t)(std::string name, std::vector<node_ptr> args);
            typedef void (__cdecl *load_t)(Interpreter*& interpreter);

            HINSTANCE hinstLib; 
            call_function_t callFuncAddress;
            load_t loadFuncAddress;

            hinstLib = LoadLibrary(TEXT(path->_Node.String().value.c_str())); 

            if (hinstLib != NULL) { 
                callFuncAddress = (call_function_t) GetProcAddress(hinstLib, "call_function");

                if (callFuncAddress == NULL) {
                    return throw_error("Error finding function 'call_function'");
                }

                loadFuncAddress = (load_t) GetProcAddress(hinstLib, "load");

                if (loadFuncAddress != NULL) {
                    loadFuncAddress(this);
                }
            }

            lib_node->_Node.Lib().handle = hinstLib;
            lib_node->_Node.Lib().call_function = callFuncAddress;

        #endif
    #else
        #if __APPLE__ || __linux__
            void* handle = dlopen(path->_Node.String().value.c_str(), RTLD_LAZY);
            
            if (!handle) {
                return throw_error("Cannot open library: " + std::string(dlerror()));
            }

            typedef node_ptr (*call_function_t)(std::string name, std::vector<node_ptr> args);
            typedef void (*load_t)(Interpreter& interpreter);

            dlerror();

            call_function_t call_function = (call_function_t) dlsym(handle, "call_function");
            const char* dlsym_error_call_function = dlerror();

            if (dlsym_error_call_function) {
                dlclose(handle);
                return throw_error("Error loading symbol 'call_function': " + std::string(dlsym_error_call_function));
            }

            load_t load = (load_t) dlsym(handle, "load");
            const char* dlsym_error_load = dlerror();

            if (!dlsym_error_load) {
                load(*this->global_interpreter);
            }

            lib_node->_Node.Lib().handle = handle;
            lib_node->_Node.Lib().call_function = call_function;

        #else

            typedef node_ptr (__cdecl *call_function_t)(std::string name, std::vector<node_ptr> args);
            typedef void (__cdecl *load_t)(Interpreter*& interpretet);

            HINSTANCE hinstLib; 
            call_function_t callFuncAddress;
            load_t loadFuncAddress;

            hinstLib = LoadLibrary(TEXT(path->_Node.String().value.c_str())); 

            if (hinstLib != NULL) { 
                callFuncAddress = (call_function_t) GetProcAddress(hinstLib, "call_function");

                if (callFuncAddress == NULL) {
                    return throw_error("Error finding function 'call_function'");
                }

                loadFuncAddress = (load_t) GetProcAddress(hinstLib, "load");

                if (loadFuncAddress != NULL) {
                    loadFuncAddress(this);
                }
            }

            lib_node->_Node.Lib().handle = hinstLib;
            lib_node->_Node.Lib().call_function = callFuncAddress;

        #endif
    #endif

    return lib_node;
}

node_ptr Interpreter::eval_call_lib_function(node_ptr& lib, node_ptr& node) {
    auto args = node->_Node.FunctionCall().args;
    if (args.size() != 2) {
        return throw_error("Library function calls expects 2 arguments");
    }

    node_ptr name = eval_node(args[0]);
    node_ptr func_args = eval_node(args[1]);

    if (name->type != NodeType::STRING) {
        return throw_error("Library function calls expects first argument to be a string");
    }

    if (func_args->type != NodeType::LIST) {
        return throw_error("Library function calls expects second argument to be a list");
    }

    return lib->_Node.Lib().call_function(name->_Node.String().value, func_args->_Node.List().elements);
}