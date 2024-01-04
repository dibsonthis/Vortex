#include <stdlib.h>
#include <filesystem>
#include "../../Vortex.hpp"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj get_env(std::string name, std::vector<VortexObj> args)
{

    if (args.size() != 1)
    {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    VortexObj arg = args[0];

    if (arg->type != NodeType::STRING)
    {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    char *value = std::getenv(arg->_Node.String().value.c_str());

    std::string env_var = std::string(value == NULL ? "" : value);
    return new_string_node(env_var);
}

VortexObj set_env(std::string name, std::vector<VortexObj> args)
{

    if (args.size() != 2)
    {
        error_and_exit("Function '" + name + "' expects 2 argument");
    }

    VortexObj _name = args[0];
    VortexObj _value = args[1];

    if (_name->type != NodeType::STRING || _value->type != NodeType::STRING)
    {
        error_and_exit("Function '" + name + "' expects 2 string arguments");
    }

    // std::string command = _name->_Node.String().value + "=" + _value->_Node.String().value;
    int res = setenv(_name->_Node.String().value.c_str(), _value->_Node.String().value.c_str(), 1);

    return new_number_node(res);
}

VortexObj os_name(std::string name, std::vector<VortexObj> args)
{

    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
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

    VortexObj os_name = new_string_node(os_name_str);

    return os_name;
}

VortexObj list_dir(std::string name, std::vector<VortexObj> args)
{

    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_path = args[0];

    if (v_path->type != NodeType::STRING)
    {
        VortexObj error = new_vortex_obj(NodeType::_ERROR);
        error->_Node.Error().message = "Parameter 'filePath' must be a string";
        return error;
    }

    VortexObj dir_list = new_vortex_obj(NodeType::LIST);

    std::error_code ec;

    for (const auto &entry : std::filesystem::directory_iterator(v_path->_Node.String().value, ec))
    {
        std::error_code ec;
        std::string path = entry.path();
        bool is_dir = std::filesystem::is_directory(path, ec);

        VortexObj filePath = new_string_node(path);
        VortexObj isDir = new_vortex_obj(NodeType::BOOLEAN);
        isDir->_Node.Boolean().value = ec ? false : is_dir;

        VortexObj res = new_vortex_obj(NodeType::OBJECT);
        res->TypeInfo.type_name = "Path";
        res->_Node.Object().properties["filePath"] = filePath;
        res->_Node.Object().properties["isDir"] = isDir;
        res->_Node.Object().keys = {"filePath", "isDir"}

                                   dir_list->_Node.List()
                                       .elements.push_back(res);
    }

    return dir_list;
}

VortexObj create_dir(std::string name, std::vector<VortexObj> args)
{

    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_path = args[0];

    if (v_path->type != NodeType::STRING)
    {
        VortexObj error = new_vortex_obj(NodeType::_ERROR);
        error->_Node.Error().message = "Parameter 'filePath' must be a string";
        return error;
    }

    bool res = std::filesystem::create_directory(v_path->_Node.String().value);
    return new_boolean_node(res);
}

VortexObj create_dirs(std::string name, std::vector<VortexObj> args)
{

    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_path = args[0];

    if (v_path->type != NodeType::STRING)
    {
        VortexObj error = new_vortex_obj(NodeType::_ERROR);
        error->_Node.Error().message = "Parameter 'filePath' must be a string";
        return error;
    }

    bool res = std::filesystem::create_directories(v_path->_Node.String().value);
    return new_boolean_node(res);
}

VortexObj remove(std::string name, std::vector<VortexObj> args)
{

    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_path = args[0];

    if (v_path->type != NodeType::STRING)
    {
        VortexObj error = new_vortex_obj(NodeType::_ERROR);
        error->_Node.Error().message = "Parameter 'filePath' must be a string";
        return error;
    }

    bool res = std::filesystem::remove(v_path->_Node.String().value);
    return new_boolean_node(res);
}

VortexObj remove_all(std::string name, std::vector<VortexObj> args)
{

    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_path = args[0];

    if (v_path->type != NodeType::STRING)
    {
        VortexObj error = new_vortex_obj(NodeType::_ERROR);
        error->_Node.Error().message = "Parameter 'filePath' must be a string";
        return error;
    }

    bool res = std::filesystem::remove_all(v_path->_Node.String().value);
    return new_boolean_node(res);
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args)
{
    if (name == "get_env")
    {
        return get_env(name, args);
    }
    if (name == "set_env")
    {
        return set_env(name, args);
    }
    if (name == "list_dir")
    {
        return list_dir(name, args);
    }
    if (name == "os_name")
    {
        return os_name(name, args);
    }
    if (name == "create_dir")
    {
        return create_dir(name, args);
    }
    if (name == "create_dirs")
    {
        return create_dirs(name, args);
    }
    if (name == "remove")
    {
        return remove(name, args);
    }
    if (name == "remove_all")
    {
        return remove_all(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}