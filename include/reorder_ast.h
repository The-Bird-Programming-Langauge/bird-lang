#pragma once
#include "ast_node/stmt/index.h"
#include "ast_node/stmt/struct_decl.h"
#include "ast_node/stmt/type_stmt.h"
#include "stack.h"
#include <memory>
#include <vector>

// This function reorders the AST to hoist structs and type stmts
void reorder_ast(std::vector<std::unique_ptr<Stmt>> &stmts);