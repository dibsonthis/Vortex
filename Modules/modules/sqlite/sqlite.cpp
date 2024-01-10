#include <sqlite3.h>
#include "include/Vortex.hpp"

std::vector<Value> results;

extern "C" Value connect(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'connect' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value v_path = args[0];

    if (!v_path.is_string())
    {
        error("Parameter 'path' must be a string");
    }

    sqlite3 *db;

    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open(v_path.get_string().c_str(), &db);

    if (rc)
    {
        error("Can't open database: " + std::string(sqlite3_errmsg(db)));
    }

    Value db_ptr = pointer_val();
    db_ptr.get_pointer()->value = db;

    return db_ptr;
}

static int callback(void *data, int argc, char **argv, char **azColName)
{

    Value columns = list_val();
    Value values = list_val();

    for (int i = 0; i < argc; i++)
    {
        columns.get_list()->push_back(string_val(std::string(azColName[i])));
        values.get_list()->push_back(string_val(std::string(argv[i] ? argv[i] : "NULL")));
    }

    Value result = object_val();
    result.get_object()->keys = {"columns", "values"};
    result.get_object()->values["columns"] = columns;
    result.get_object()->values["values"] = values;

    results.push_back(result);
    return 0;
}

extern "C" Value execute(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'execute' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    results.clear();

    Value v_db = args[0];
    Value v_stmnt = args[1];

    if (!v_db.is_pointer())
    {
        error("Parameter 'dbPtr' must be a pointer");
    }
    if (!v_stmnt.is_string())
    {
        error("Parameter 'statement' must be a string");
    }

    sqlite3 *db = (sqlite3 *)v_db.get_pointer()->value;

    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_exec(db, v_stmnt.get_string().c_str(), callback, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        std::string error_msg = "SQL Error: " + std::string(zErrMsg);
        sqlite3_free(zErrMsg);
        error(error_msg);
    }

    Value res_list = list_val();
    *res_list.get_list() = results;

    return res_list;
}

extern "C" Value close(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'execute' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    results.clear();

    Value v_db = args[0];

    if (!v_db.is_pointer())
    {
        error("Parameter 'dbPtr' must be a pointer");
    }

    sqlite3 *db = (sqlite3 *)v_db.get_pointer()->value;

    sqlite3_close(db);

    return none_val();
}