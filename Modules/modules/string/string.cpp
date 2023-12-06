#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include "include/Vortex.hpp"

extern "C" Value split(std::vector<Value>& args) {
    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error("Function 'split' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value& text = args[0];
    Value& delimiter = args[1];

    if (!text.is_string() || !delimiter.is_string()) {
        error("Function 'split' expects args 'text', 'delimiter' to be strings");
    }

    std::string& str = text.get_string();
    std::string delim = delimiter.get_string();

    if (delim == "") {
        delim = " ";
    }

    size_t pos_start = 0, pos_end, delim_len = delim.length();
    std::string token;

    Value res = list_val();

    while ((pos_end = str.find(delim, pos_start)) != std::string::npos) {
        token = str.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;

        Value str_val = string_val(token);

        res.get_list()->push_back(str_val);
    }

    auto last_str_val = string_val(str.substr(pos_start));
    res.get_list()->push_back(last_str_val);

    return res;
}

extern "C" Value trim(std::vector<Value>& args) {
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error("Function 'trim' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value& text = args[0];

    if (!text.is_string()) {
        error("Function 'chars' expects argument 'text' to be a string");
    }
    
    Value new_string = string_val(text.get_string());

    new_string.get_string().erase(new_string.get_string().begin(), std::find_if(new_string.get_string().begin(), new_string.get_string().end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));

    new_string.get_string().erase(std::find_if(new_string.get_string().rbegin(), new_string.get_string().rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), new_string.get_string().end());

    return new_string;
}

extern "C" Value chars(std::vector<Value>& args) {
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error("Function 'chars' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value& text = args[0];
    Value list = list_val();

    if (!text.is_string()) {
        error("Function 'chars' expects argument 'text' to be a string");
    }

    for (auto c : text.get_string()) {
        list.get_list()->push_back(string_val(std::string(1, c)));
    }

    return list;
}

extern "C" Value replaceAll(std::vector<Value>& args) {
    int num_required_args = 3;

    if (args.size() != num_required_args) {
        error("Function 'replaceAll' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value& _str = args[0];
    Value& _from = args[1];
    Value& _to = args[2];

    if (!_str.is_string() || !_from.is_string() || !_to.is_string()) {
        error("Function 'replaceAll' expects " + std::to_string(num_required_args) + " string argument(s)");
    }

    std::string& from = _from.get_string();
    std::string& to = _to.get_string();
    std::string& str = _str.get_string();

    Value new_str = string_val(str);

    if(from.empty()) {
        return new_str;
    }

    size_t start_pos = 0;
    while((start_pos = new_str.get_string().find(from, start_pos)) != std::string::npos) {
        new_str.get_string().replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return new_str;
}