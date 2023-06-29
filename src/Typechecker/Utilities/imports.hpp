#pragma once
#include "../Typechecker.hpp"
#include "../../Lexer/Lexer.hpp"
#include "../../Parser/Parser.hpp"
#include "../../utils/utils.hpp"

node_ptr Typechecker::tc_import(node_ptr& node) {

    if (node->_Node.Import().is_default) {
        node->_Node.Import().target = new_string_node("@modules/" + node->_Node.Import().module->_Node.ID().value);
    }

    if (node->_Node.Import().target->type == NodeType::ID) {
        node->_Node.Import().target = new_string_node("@modules/" + node->_Node.Import().target->_Node.ID().value);
    }

    if (node->_Node.Import().target->type != NodeType::STRING) {
        return throw_error("Import target must be a string");
    }

    std::string target_name = std::string(node->_Node.Import().target->_Node.String().value);
    replaceAll(target_name, "@modules/", "");

    std::string path = node->_Node.Import().target->_Node.String().value + ".vtx";

    #if GCC_COMPILER
        #if __apple__ || __linux__
            replaceAll(path, "@modules", "/usr/local/share/vortex/modules/" + target_name);
        #else
            replaceAll(path, "@modules", "C:/Program Files/Vortex/modules/" + target_name);
        #endif
    #else
        #if defined(__APPLE__) || defined(__linux__)
            replaceAll(path, "@modules", "/usr/local/share/vortex/modules/" + target_name);
        #else
            replaceAll(path, "@modules", "C:/Program Files/Vortex/modules/" + target_name);
        #endif
    #endif

    if (node->_Node.Import().module->type == NodeType::ID) {
        std::string module_name = node->_Node.Import().module->_Node.ID().value;
        node_ptr import_obj = new_node(NodeType::OBJECT);

        Lexer import_lexer(path);
        import_lexer.file_name = path;
        import_lexer.tokenize();

        Parser import_parser(import_lexer.nodes, import_lexer.file_name);
        import_parser.parse(0, "_");
        import_parser.remove_op_node(";");
        
        auto current_path = std::filesystem::current_path();
        auto parent_path = std::filesystem::path(path).parent_path();
        try {
            if (parent_path != "") {
                std::filesystem::current_path(parent_path);
            }
        } catch(...) {
            return throw_error("No such file or directory: '" + parent_path.string() + "'");
        }

        Typechecker import_Typechecker(import_parser.nodes, import_parser.file_name);
        import_Typechecker.global_interpreter = this;
        import_Typechecker.typecheck();

        std::filesystem::current_path(current_path);

        for (auto& symbol : import_Typechecker.current_scope->symbols) {
            import_obj->_Node.Object().properties[symbol.first] = symbol.second;
        }

        add_symbol(module_name, import_obj, current_scope);
        return new_node(NodeType::NONE);
    }

    if (node->_Node.Import().module->type == NodeType::LIST) {
        node_ptr import_module = node->_Node.Import().module;
        // Before we import, we'll check to see if the import list
        // contains only IDs
        if (import_module->_Node.List().elements.size() == 1 && import_module->_Node.List().elements[0]->type == NodeType::COMMA_LIST) {
            import_module = import_module->_Node.List().elements[0];
        }
        for (node_ptr elem : import_module->_Node.List().elements) {
            if (elem->type != NodeType::ID) {
                return throw_error("Import list must contain identifiers");
            }
        }

        Lexer import_lexer(path);
        import_lexer.file_name = path;
        import_lexer.tokenize();

        Parser import_parser(import_lexer.nodes, import_lexer.file_name);
        import_parser.parse(0, "_");
        import_parser.remove_op_node(";");

        auto current_path = std::filesystem::current_path();
        auto parent_path = std::filesystem::path(path).parent_path();
        try {
            if (parent_path != "") {
                std::filesystem::current_path(parent_path);
            }
        } catch(...) {
            return throw_error("No such file or directory: '" + parent_path.string() + "'");
        }

        Typechecker import_Typechecker(import_parser.nodes, import_parser.file_name);
        import_Typechecker.global_interpreter = this;
        import_Typechecker.typecheck();

        std::filesystem::current_path(current_path);

        import_Typechecker.current_scope->parent = import_Typechecker.global_scope;

        std::unordered_map<std::string, node_ptr> imported_variables;

        for (auto& symbol : import_Typechecker.current_scope->symbols) {
            imported_variables[symbol.first] = symbol.second;
        }

        if (import_module->_Node.List().elements.size() == 0) {
            for (auto& elem : import_Typechecker.current_scope->symbols) {
                add_symbol(elem.first, elem.second, current_scope);
            }

            return new_node(NodeType::NONE);
        }

        for (node_ptr elem : import_module->_Node.List().elements) {
            if (imported_variables.contains(elem->_Node.ID().value)) {
                add_symbol(elem->_Node.ID().value, import_Typechecker.current_scope->symbols[elem->_Node.ID().value], current_scope);
            } else {
                return throw_error("Cannot import value '" + elem->_Node.ID().value + "' - variable undefined");
            }
        }

        return new_node(NodeType::NONE);
    }

    return throw_error("Malformed import statement");
}