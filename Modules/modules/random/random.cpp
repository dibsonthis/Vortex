#include <random>
#include "include/Vortex.hpp"

extern "C" Value _rand(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        return error_object("Function 'rand' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    const int range_from = -INT_MAX;
    const int range_to = INT_MAX;
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<int> distr(range_from, range_to);

    return number_val(distr(generator));
}

extern "C" Value _rand_range(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        return error_object("Function 'rand_range' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value start = args[0];
    Value end = args[1];

    if (!start.is_number())
    {
        return error_object("Parameter 'start' must be a number");
    }
    if (!end.is_number())
    {
        return error_object("Parameter 'end' must be a number");
    }

    const int range_from = args[0].get_number();
    const int range_to = args[1].get_number();
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<int> distr(range_from, range_to);

    return number_val(distr(generator));
}