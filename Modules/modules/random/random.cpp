#include <cstdlib>
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj random_number(std::string name, void* handle, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }

    int random_num = std::rand();
    VortexObj num_obj = new_vortex_obj(NodeType::NUMBER);
    num_obj->Number.value = random_num;
    return num_obj;

}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, void* handle, std::vector<VortexObj> args) {
    if (name == "random_number") {
        return random_number(name, handle, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}