#include <stdlib.h>
#include <filesystem>
#include "include/Vortex.hpp"

extern "C" Value list_dir(std::vector<Value>& args) {
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error("Function 'list_dir' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value filePath = args[0];

    if (!filePath.is_string()) {
        error("Parameter 'filePath' must be a string");
    }

    Value dir_list = list_val();

    std::error_code ec;

    for (const auto& entry : std::filesystem::directory_iterator(filePath.get_string(), ec)) {
        std::error_code ec;
        std::string path = entry.path();
        bool is_dir = std::filesystem::is_directory(path, ec);

        Value _path = string_val(path);
        Value isDir = boolean_val(ec ? false : is_dir);

        Value res = object_val();
        res.get_object()->values["filePath"] = _path;
        res.get_object()->values["isDir"] = isDir;

        dir_list.get_list()->push_back(res);
    }

    return dir_list;
}

extern "C" Value os_name(std::vector<Value>& args) {
    int num_required_args = 0;

    if (args.size() != num_required_args) {
        error("Function 'os_name' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    std::string os_name_str;

    #ifdef _WIN32
    os_name_str = "Windows 32-bit";
    #elif _WIN64
    os_name_str = "Windows 64-bit";
    #elif __APPLE__ || __MACH__
    os_name_str = "Mac OSX";
    #elif __linux__
    os_name_str = "Linux";
    #elif __FreeBSD__
    os_name_str = "FreeBSD";
    #elif __unix || __unix__
    os_name_str = "Unix";
    #else
    os_name_str = "Other";
    #endif

    Value os_name = string_val(os_name_str);
    return os_name;
}