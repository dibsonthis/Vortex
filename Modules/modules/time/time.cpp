#include <iostream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <string>
#include <thread>
#include "include/Vortex.hpp"

extern "C" Value datetime(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'datetime' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    auto curr_time = std::chrono::system_clock::now();
    std::time_t pcurr_time = std::chrono::system_clock::to_time_t(curr_time);

    std::string datetime = std::string(std::ctime(&pcurr_time));
    datetime.erase(std::remove(datetime.begin(), datetime.end(), '\n'), datetime.cend());

    return string_val(datetime);
}

extern "C" Value sleep(std::vector<Value> &args)
{
    using namespace std::chrono_literals;

    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'sleep' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value ms = args[0];

    if (!ms.is_number())
    {
        error("Function 'sleep' expects argument 'ms' to be a number");
    }

    auto _ms = ms.get_number() * 1ms;

    std::this_thread::sleep_for(_ms);

    return none_val();
}