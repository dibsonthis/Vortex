#pragma once
#include "Bytecode.hpp"
#include "../Node/Node.hpp"

void gen_literal(Chunk& chunk, node_ptr node);
void gen_neg(Chunk& chunk, node_ptr node);
void gen_add(Chunk& chunk, node_ptr node);
void gen_multiply(Chunk& chunk, node_ptr node);
void gen_subtract(Chunk& chunk, node_ptr node);
void gen_divide(Chunk& chunk, node_ptr node);
void gen_not(Chunk& chunk, node_ptr node);
void generate(node_ptr node, Chunk& chunk);
void generate_bytecode(std::vector<node_ptr>& nodes, Chunk& chunk);

