
#include "include/Vortex.hpp"

extern "C" Value add(std::vector<Value>& args) {
    Value a = args[0];
    Value b = args[1];

    return number_val(a.get_number() + b.get_number());
}