#pragma once
#include "Bytecode.hpp"
#include "../Node/Node.hpp"

struct StackFrame {
    std::string name;
    std::vector<std::string> local_var_names;
};

struct Local {
    std::string name;
    int depth;
};

struct Environment {
    std::vector<Local> locals;
    int depth;
};

void gen_literal(Chunk& chunk, node_ptr node);
void gen_neg(Chunk& chunk, node_ptr node);
void gen_add(Chunk& chunk, node_ptr node);
void gen_multiply(Chunk& chunk, node_ptr node);
void gen_subtract(Chunk& chunk, node_ptr node);
void gen_divide(Chunk& chunk, node_ptr node);
void gen_not(Chunk& chunk, node_ptr node);
void gen_eq_eq(Chunk& chunk, node_ptr node);
void gen_not_eq(Chunk& chunk, node_ptr node);
void gen_lt_eq(Chunk& chunk, node_ptr node);
void gen_gt_eq(Chunk& chunk, node_ptr node);
void gen_lt(Chunk& chunk, node_ptr node);
void gen_gt(Chunk& chunk, node_ptr node);

void gen_paren(Chunk& chunk, node_ptr node);
void gen_var(Chunk& chunk, node_ptr node);
void gen_id(Chunk& chunk, node_ptr node);

void generate(node_ptr node, Chunk& chunk);
void generate_bytecode(std::vector<node_ptr>& nodes, Chunk& chunk);

static void add_local(std::string name);
static int resolve_local(Environment& env, std::string name);

