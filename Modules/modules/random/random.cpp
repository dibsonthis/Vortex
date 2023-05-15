#include <random>
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj rand(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }

    const int range_from  = -INT_MAX;
    const int range_to    = INT_MAX;
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>  distr(range_from, range_to);

    return new_number_node(distr(generator));
}

VortexObj rand_range(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 2 arguments");
    }

    if (args[0]->type != NodeType::NUMBER || args[1]->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 2 number arguments");
    }

    const int range_from  = args[0]->_Node.Number().value;
    const int range_to    = args[1]->_Node.Number().value;
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>  distr(range_from, range_to);

    return new_number_node(distr(generator));
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "rand") {
        return rand(name, args);
    }
    if (name == "rand_range") {
        return rand_range(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}