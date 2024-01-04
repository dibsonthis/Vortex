#include <cmath>
#include "include/Vortex.hpp"

extern "C" Value ceil_(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'ceil' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    double new_value = std::ceil(value.get_number());
    return number_val(new_value);
}

extern "C" Value floor_(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'floor' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    double new_value = std::floor(value.get_number());
    return number_val(new_value);
}

extern "C" Value abs_(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'abs' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    double new_value = std::abs(value.get_number());
    return number_val(new_value);
}

extern "C" Value sqrt_(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'sqrt' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    double new_value = std::sqrt(value.get_number());
    return number_val(new_value);
}

extern "C" Value trunc_(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'trunc' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    double num = value.get_number();
    double new_value;

    if (num > 0)
    {
        new_value = std::floor(num);
    }
    else if (num < 0)
    {
        new_value = std::ceil(num);
    }
    else
    {
        new_value = 0;
    }

    return number_val(new_value);
}

extern "C" Value log_(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'log' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];
    Value base = args[1];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    if (!base.is_number())
    {
        error("Parameter 'base' must be a number");
    }

    double new_value = std::log2(value.get_number()) / std::log2(base.get_number());

    return number_val(new_value);
}

extern "C" Value pow_(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'pow' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];
    Value exponent = args[1];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    if (!exponent.is_number())
    {
        error("Parameter 'exponent' must be a number");
    }

    double new_value = std::pow(value.get_number(), exponent.get_number());

    return number_val(new_value);
}

/* Trig Functions */

extern "C" Value tan_(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'tan' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    auto new_value = std::tan(value.get_number());
    return number_val(new_value);
}

extern "C" Value sin_(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'sin' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    auto new_value = std::sin(value.get_number());
    return number_val(new_value);
}

extern "C" Value cos_(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'cos' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value value = args[0];

    if (!value.is_number())
    {
        error("Parameter 'value' must be a number");
    }

    auto new_value = std::cos(value.get_number());
    return number_val(new_value);
}

extern "C" Value multMat4(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'multMat4' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value vec = args[0];
    Value mat = args[1];

    if (!vec.is_object())
    {
        error("Parameter 'vec' must be an object");
    }

    auto &in = vec.get_object();

    if (!mat.is_list())
    {
        error("Parameter 'mat' must be an object");
    }

    auto &list = mat.get_list();

    Value vecOut = object_val();
    auto &out = vecOut.get_object();
    out->values["x"] = number_val(
        in->values["x"].get_number() * list->at(0).get_list()->at(0).get_number() +
        in->values["y"].get_number() * list->at(1).get_list()->at(0).get_number() +
        in->values["z"].get_number() * list->at(2).get_list()->at(0).get_number() +
        list->at(3).get_list()->at(0).get_number());

    out->values["y"] = number_val(
        in->values["x"].get_number() * list->at(0).get_list()->at(1).get_number() +
        in->values["y"].get_number() * list->at(1).get_list()->at(1).get_number() +
        in->values["z"].get_number() * list->at(2).get_list()->at(1).get_number() +
        list->at(3).get_list()->at(1).get_number());

    out->values["z"] = number_val(
        in->values["x"].get_number() * list->at(0).get_list()->at(2).get_number() +
        in->values["y"].get_number() * list->at(1).get_list()->at(2).get_number() +
        in->values["z"].get_number() * list->at(2).get_list()->at(2).get_number() +
        list->at(3).get_list()->at(2).get_number());

    double w =
        in->values["x"].get_number() * list->at(0).get_list()->at(3).get_number() +
        in->values["y"].get_number() * list->at(1).get_list()->at(3).get_number() +
        in->values["z"].get_number() * list->at(2).get_list()->at(3).get_number() +
        list->at(3).get_list()->at(3).get_number();

    if (w != 0)
    {
        out->values["x"] = number_val(out->values["x"].get_number() / w);
        out->values["y"] = number_val(out->values["y"].get_number() / w);
        out->values["z"] = number_val(out->values["z"].get_number() / w);
    }

    vecOut.get_object()->keys = {"x",
                                 "y",
                                 "z"};

    return vecOut;
}

// /* Trig Functions */

// VortexObj acos_(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::acos(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj asin(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::asin(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj atan(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::atan(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj atan2(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 2;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];
//     VortexObj value_b = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value_a' must be a number";
//         return error;
//     }

//     if (value_b->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value_b' must be a number";
//         return error;
//     }

//     auto new_value = std::atan2(value->_Node.Number().value, value_b->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj sin(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::sin(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj cos(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::cos(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// /* Angular Conversion */

// VortexObj deg(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = value->_Node.Number().value * (180/M_PI);
//     return new_number_node(new_value);
// }

// VortexObj rad(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = value->_Node.Number().value * (M_PI/180);
//     return new_number_node(new_value);
// }

// /* Hyperbolic Functions */

// VortexObj acosh(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::acosh(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj asinh(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::asinh(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj atanh(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::atanh(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj sinh(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::sinh(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj cosh(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::cosh(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// VortexObj tanh(std::string name, std::vector<VortexObj> args) {

//     int num_required_args = 1;

//     if (args.size() != num_required_args) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
//     }

//     VortexObj value = args[0];

//     if (value->type != NodeType::NUMBER) {
//         VortexObj error = new_vortex_obj(NodeType::_ERROR);
//         error->_Node.Error().message = "Parameter 'value' must be a number";
//         return error;
//     }

//     auto new_value = std::tanh(value->_Node.Number().value);
//     return new_number_node(new_value);
// }

// /* Implement call_function */

// extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
//     if (name == "ceil") {
//         return ceil(name, args);
//     }
//     if (name == "floor") {
//         return floor(name, args);
//     }
//     if (name == "abs") {
//         return abs(name, args);
//     }
//     if (name == "sqrt") {
//         return sqrt(name, args);
//     }
//     if (name == "trunc") {
//         return trunc(name, args);
//     }
//     if (name == "log") {
//         return log(name, args);
//     }
//     if (name == "pow") {
//         return pow(name, args);
//     }
//     if (name == "asin") {
//         return asin(name, args);
//     }
//     if (name == "acos") {
//         return acos(name, args);
//     }
//     if (name == "atan") {
//         return atan(name, args);
//     }
//     if (name == "atan2") {
//         return atan2(name, args);
//     }
//     if (name == "sin") {
//         return sin(name, args);
//     }
//     if (name == "cos") {
//         return cos(name, args);
//     }
//     if (name == "tan") {
//         return tan(name, args);
//     }
//     if (name == "deg") {
//         return deg(name, args);
//     }
//     if (name == "rad") {
//         return rad(name, args);
//     }
//     if (name == "acosh") {
//         return acosh(name, args);
//     }
//     if (name == "asinh") {
//         return asinh(name, args);
//     }
//     if (name == "atanh") {
//         return atanh(name, args);
//     }
//     if (name == "sinh") {
//         return sinh(name, args);
//     }
//     if (name == "cosh") {
//         return cosh(name, args);
//     }
//     if (name == "tanh") {
//         return tanh(name, args);
//     }

//     error_and_exit("Function '" + name + "' is undefined");

//     return new_vortex_obj(NodeType::NONE);
// }