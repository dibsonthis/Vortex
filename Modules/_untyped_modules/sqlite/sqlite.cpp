#include <sqlite3.h>
#include "../../Vortex.hpp"

/* Define Vars */

std::vector<VortexObj> results;

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj init(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 0;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    sqlite3 *db;
    VortexObj db_ptr = new_vortex_obj(NodeType::POINTER);
    db_ptr->_Node.Pointer().value = db;

    return db_ptr;
}

VortexObj connect(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_path = args[0];

    if (v_path->type != NodeType::STRING) {
        VortexObj error = new_vortex_obj(NodeType::_ERROR);
        error->_Node.Error().message = "Parameter 'filePath' must be a string";
        return error;
    }

    sqlite3 *db;

    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open(v_path->_Node.String().value.c_str(), &db);

    if (rc) {
        return new_error_node("Can't open database: " + std::string(sqlite3_errmsg(db)));
    }

    VortexObj db_ptr = new_vortex_obj(NodeType::POINTER);
    db_ptr->_Node.Pointer().value = db;

    return db_ptr;
}

static int callback(void *data, int argc, char **argv, char **azColName) {

    VortexObj columns = new_vortex_obj(NodeType::LIST);
    VortexObj values = new_vortex_obj(NodeType::LIST);
    
    for(int i = 0; i < argc; i++) {
        columns->_Node.List().elements.push_back(new_string_node(std::string(azColName[i])));
        values->_Node.List().elements.push_back(new_string_node(std::string(argv[i] ? argv[i] : "NULL")));
    }

    VortexObj result = new_vortex_obj(NodeType::OBJECT);
    result->_Node.Object().properties["columns"] = columns;
    result->_Node.Object().properties["values"] = values;

    results.push_back(result);
    return 0;
}

VortexObj execute(std::string name, std::vector<VortexObj> args) {

    results.clear();

    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_db = args[0];
    VortexObj v_stmnt = args[1];

    if (v_db->type != NodeType::POINTER) {
        VortexObj error = new_vortex_obj(NodeType::_ERROR);
        error->_Node.Error().message = "Parameter 'dbPtr' must be a string";
        return error;
    }

    if (v_stmnt->type != NodeType::STRING) {
        VortexObj error = new_vortex_obj(NodeType::_ERROR);
        error->_Node.Error().message = "Parameter 'statement' must be a string";
        return error;
    }

    sqlite3* db = (sqlite3*)v_db->_Node.Pointer().value;

    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_exec(db, v_stmnt->_Node.String().value.c_str(), callback, 0, &zErrMsg);

    if (rc != SQLITE_OK){
        VortexObj error = new_error_node("SQL Error: " + std::string(zErrMsg));
        sqlite3_free(zErrMsg);
        return error;
    }

    VortexObj res_list = new_vortex_obj(NodeType::LIST);
    res_list->_Node.List().elements = results;

    return res_list;
}

VortexObj close(std::string name, std::vector<VortexObj> args) {

    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error_and_exit("Function '" + name + "' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    VortexObj v_db = args[0];

    if (v_db->type != NodeType::POINTER) {
        VortexObj error = new_vortex_obj(NodeType::_ERROR);
        error->_Node.Error().message = "Parameter 'dbPtr' must be a string";
        return error;
    }

    sqlite3* db = (sqlite3*)v_db->_Node.Pointer().value;

    sqlite3_close(db);

    return new_vortex_obj(NodeType::NONE);
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "init") {
        return init(name, args);
    }
    if (name == "connect") {
        return connect(name, args);
    }
    if (name == "execute") {
        return execute(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}