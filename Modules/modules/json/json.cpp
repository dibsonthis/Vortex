#include "include/json.hpp"
#include "include/Vortex.hpp"

int indent_level = 0;
std::string indent = "    ";

std::string toString(Value value, bool quote_strings = false)
{
    switch (value.type)
    {
    case Number:
    {
        double number = value.get_number();
        if (std::floor(number) == number)
        {
            return std::to_string((long long)number);
        }
        char buff[100];
        snprintf(buff, sizeof(buff), "%.8g", number);
        std::string buffAsStdStr = buff;
        return buffAsStdStr;
    }
    case String:
    {
        if (quote_strings)
        {
            return "\"" + value.get_string() + "\"";
        }
        return (value.get_string());
    }
    case Boolean:
    {
        return (value.get_boolean() ? "true" : "false");
    }
    case List:
    {
        std::string repr = "[";
        for (int i = 0; i < value.get_list()->size(); i++)
        {
            Value &v = value.get_list()->at(i);
            repr += toString(v, quote_strings);
            if (i < value.get_list()->size() - 1)
            {
                repr += ", ";
            }
        }
        repr += "]";
        return repr;
    }
    case Object:
    {
        auto &obj = value.get_object();
        std::string repr;
        if (obj->type)
        {
            repr += value.get_object()->type->name + " ";
        }
        repr += "{ ";
        int i = 0;
        int size = obj->values.size();
        for (std::string &key : obj->keys)
        {
            if (quote_strings)
            {
                key = "\"" + key + "\"";
            }
            repr += key + ": " + toString(obj->values[key], quote_strings);
            i++;
            if (i < size)
            {
                repr += ", ";
            }
        }
        repr += " }";
        return repr;
    }
    case None:
    {
        return "null";
    }
    default:
    {
        return "null";
    }
    }
}

Value json_type_to_vtx_type(nlohmann::json_abi_v3_11_2::ordered_json json_obj)
{

    auto type = json_obj.type();

    if (type == nlohmann::ordered_json::value_t::string)
    {
        return string_val(json_obj);
    }
    if (type == nlohmann::ordered_json::value_t::boolean)
    {
        return boolean_val(json_obj);
    }
    if (type == nlohmann::ordered_json::value_t::number_integer || type == nlohmann::ordered_json::value_t::number_unsigned || type == nlohmann::ordered_json::value_t::number_float)
    {
        return number_val(json_obj);
    }
    if (type == nlohmann::ordered_json::value_t::null)
    {
        return none_val();
    }
    if (type == nlohmann::ordered_json::value_t::array)
    {
        Value list = list_val();
        for (auto val : json_obj)
        {
            list.get_list()->push_back(json_type_to_vtx_type(val));
        }
        return list;
    }
    if (type == nlohmann::ordered_json::value_t::object)
    {
        Value object = object_val();
        for (auto val : json_obj.items())
        {
            object.get_object()->values[val.key()] = json_type_to_vtx_type(val.value());
            object.get_object()->keys.push_back(val.key());
        }
        return object;
    }

    return none_val();
}

extern "C" Value parse(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'parse' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value jsonStr = args[0];

    if (!jsonStr.is_string())
    {
        error("Parameter 'jsonStr' must be a string");
    }

    std::string json_body = jsonStr.get_string();

    try
    {
        auto json = nlohmann::ordered_json::parse(json_body);
        // If it's a list
        if (json.type() == nlohmann::json::value_t::array)
        {
            Value json_obj = list_val();
            for (auto &prop : json.items())
            {
                auto value = prop.value();
                json_obj.get_list()->push_back(json_type_to_vtx_type(value));
            }
            return json_obj;
        }
        // Otherwise it's an object
        Value json_obj = object_val();
        for (auto &prop : json.items())
        {
            auto value = prop.value();
            auto key = prop.key();
            json_obj.get_object()->values[key] = json_type_to_vtx_type(value);
            json_obj.get_object()->keys.push_back(key);
        }
        return json_obj;
    }
    catch (const std::exception &exc)
    {
        return object_val();
    }
}

extern "C" Value serialize(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'serialize' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value jsonObj = args[0];

    if (!jsonObj.is_list() && !jsonObj.is_object())
    {
        error("Parameter 'jsonObj' must be a list or object");
    }

    Value serialized_object = string_val(toString(jsonObj, true));
    return serialized_object;
}