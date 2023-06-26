#include <random>
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj ceil(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::ceil(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj floor(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::floor(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj abs(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::abs(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj sqrt(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::sqrt(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj trunc(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    double new_value;
    
    if (value > 0) {
        new_value = std::floor(value->_Node.Number().value);
    } else if (value < 0) {
        new_value = std::ceil(value->_Node.Number().value);
    } else {
        new_value = 0;
    }

    return new_number_node(new_value);
}

VortexObj log(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];
    VortexObj base = args[1];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    if (base->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'base' must be a number";
        return error;
    }

    auto new_value = std::log2(value->_Node.Number().value) / std::log2(base->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj pow(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];
    VortexObj exponent = args[1];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    if (exponent->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'exponent' must be a number";
        return error;
    }

    auto new_value = std::pow(value->_Node.Number().value, exponent->_Node.Number().value);
    return new_number_node(new_value);
}

/* Trig Functions */

VortexObj acos(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::acos(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj asin(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::asin(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj atan(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::atan(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj atan2(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];
    VortexObj value_b = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value_a' must be a number";
        return error;
    }

    if (value_b->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value_b' must be a number";
        return error;
    }

    auto new_value = std::atan2(value->_Node.Number().value, value_b->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj sin(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::sin(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj cos(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::cos(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj tan(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::tan(value->_Node.Number().value);
    return new_number_node(new_value);
}

/* Angular Conversion */

VortexObj deg(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = value->_Node.Number().value * (180/M_PI);
    return new_number_node(new_value);
}

VortexObj rad(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = value->_Node.Number().value * (M_PI/180);
    return new_number_node(new_value);
}

/* Hyperbolic Functions */

VortexObj acosh(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::acosh(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj asinh(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::asinh(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj atanh(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::atanh(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj sinh(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::sinh(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj cosh(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::cosh(value->_Node.Number().value);
    return new_number_node(new_value);
}

VortexObj tanh(std::string name, std::vector<VortexObj> args) {
    
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj value = args[0];

    if (value->type != NodeType::NUMBER) {
        VortexObj error = new_vortex_obj(NodeType::ERROR);
        error->_Node.Error().message = "Parameter 'value' must be a number";
        return error;
    }

    auto new_value = std::tanh(value->_Node.Number().value);
    return new_number_node(new_value);
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "ceil") {
        return ceil(name, args);
    }
    if (name == "floor") {
        return floor(name, args);
    }
    if (name == "abs") {
        return abs(name, args);
    }
    if (name == "sqrt") {
        return sqrt(name, args);
    }
    if (name == "trunc") {
        return trunc(name, args);
    }
    if (name == "log") {
        return log(name, args);
    }
    if (name == "pow") {
        return pow(name, args);
    }
    if (name == "asin") {
        return asin(name, args);
    }
    if (name == "acos") {
        return acos(name, args);
    }
    if (name == "atan") {
        return atan(name, args);
    }
    if (name == "atan2") {
        return atan2(name, args);
    }
    if (name == "sin") {
        return sin(name, args);
    }
    if (name == "cos") {
        return cos(name, args);
    }
    if (name == "tan") {
        return tan(name, args);
    }
    if (name == "deg") {
        return deg(name, args);
    }
    if (name == "rad") {
        return rad(name, args);
    }
    if (name == "acosh") {
        return acosh(name, args);
    }
    if (name == "asinh") {
        return asinh(name, args);
    }
    if (name == "atanh") {
        return atanh(name, args);
    }
    if (name == "sinh") {
        return sinh(name, args);
    }
    if (name == "cosh") {
        return cosh(name, args);
    }
    if (name == "tanh") {
        return tanh(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}