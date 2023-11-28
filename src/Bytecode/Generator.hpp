#pragma once
#include "../utils/utils.hpp"
#include "Bytecode.hpp"
#include "../Node/Node.hpp"

struct Variable
{
    std::string name;
    int depth = 0;
    bool is_const = false;
    bool is_internal = false;
};

struct Compiler
{
    std::string name;
    bool root = true;
    std::vector<Variable> variables;
    int variableCount = 0;
    int scopeDepth = 0;
    bool in_loop = false;
    bool in_function = false;
    bool in_object = false;
    int nested_object_count = 0;
    // std::vector<int> closed_vars;
    std::vector<ClosedVar> closed_vars;
    std::shared_ptr<Compiler> prev;
};

void gen_literal(Chunk &chunk, node_ptr node);
void gen_neg(Chunk &chunk, node_ptr node);
void gen_add(Chunk &chunk, node_ptr node);
void gen_multiply(Chunk &chunk, node_ptr node);
void gen_subtract(Chunk &chunk, node_ptr node);
void gen_divide(Chunk &chunk, node_ptr node);
void gen_mod(Chunk &chunk, node_ptr node);
void gen_bin_and(Chunk &chunk, node_ptr node);
void gen_bin_or(Chunk &chunk, node_ptr node);
void gen_not(Chunk &chunk, node_ptr node);
void gen_eq_eq(Chunk &chunk, node_ptr node);
void gen_not_eq(Chunk &chunk, node_ptr node);
void gen_lt_eq(Chunk &chunk, node_ptr node);
void gen_gt_eq(Chunk &chunk, node_ptr node);
void gen_lt(Chunk &chunk, node_ptr node);
void gen_gt(Chunk &chunk, node_ptr node);
void gen_eq(Chunk &chunk, node_ptr node);
void gen_range(Chunk &chunk, node_ptr node);

void gen_paren(Chunk &chunk, node_ptr node);
void gen_var(Chunk &chunk, node_ptr node);
void gen_const(Chunk &chunk, node_ptr node);
void gen_id(Chunk &chunk, node_ptr node, int global_flag = 0);
int gen_if(Chunk &chunk, node_ptr node);
void gen_if_block(Chunk &chunk, node_ptr node);
void gen_while_loop(Chunk &chunk, node_ptr node);
void gen_for_loop(Chunk &chunk, node_ptr node);
void gen_and(Chunk &chunk, node_ptr node);
void gen_or(Chunk &chunk, node_ptr node);
void gen_break(Chunk &chunk, node_ptr node);
void gen_continue(Chunk &chunk, node_ptr node);
void gen_function(Chunk &chunk, node_ptr node);
void gen_function_call(Chunk &chunk, node_ptr node);
void gen_type(Chunk &chunk, node_ptr node);
void gen_typed_object(Chunk &chunk, node_ptr node);
void gen_object(Chunk &chunk, node_ptr node);
void gen_dot(Chunk &chunk, node_ptr node);
void gen_hook(Chunk &chunk, node_ptr node);
void gen_import(Chunk &chunk, node_ptr node);

void gen_list(Chunk &chunk, node_ptr node);

void generate(node_ptr node, Chunk &chunk);
void generate_bytecode(std::vector<node_ptr> &nodes, Chunk &chunk);

static void begin_scope();
static void end_scope(Chunk &chunk);

static void addVariable(std::string name, bool is_const, bool is_internal = false);
static void removeVariable(std::string name);

static void declareVariable(std::string name, bool is_const = false, bool is_internal = false);

static int resolve_variable(std::string name);
static int resolve_closure(std::string name);
static int resolve_closure_nested(std::string name);

void error(std::string message);
void reset();
