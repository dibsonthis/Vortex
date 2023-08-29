#include <fstream>
#include <sstream>
#include <iostream>
#include "include/Vortex.hpp"

extern "C" Value rename_(std::vector<Value>& args) {
    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error("Function 'rename' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value function = args[0];
    Value name = args[1];

    if (!function.is_function()) {
        error("Parameter 'function' must be a function");
    }

    if (!name.is_string()) {
        error("Parameter 'name' must be a string");
    }

    Value new_func = function;
    new_func.get_function()->name = name.get_string();
    return new_func;
}