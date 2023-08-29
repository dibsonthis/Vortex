#include <fstream>
#include <sstream>
#include <iostream>
#include "include/Vortex.hpp"

extern "C" Value split(std::vector<Value>& args) {
    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error("Function 'split' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value& text = args[0];
    Value& delimiter = args[1];

    if (!text.is_string() || !delimiter.is_string()) {
        error("Function 'createWindow' expects args 'text', 'delimiter' to be strings");
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

    for (auto c : text.get_string()) {
        list.get_list()->push_back(string_val(std::string(1, c)));
    }

    return list;
}