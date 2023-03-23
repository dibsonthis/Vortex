#include "utils.hpp"

bool vector_contains_string(std::vector<std::string> vec, std::string value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}