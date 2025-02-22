#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"

/*
 * Break statement class AST node that
 * represents loop breaks
 * ex:
 * while (true) {
 *     break;
 * }
 */
class BreakStmt : public Stmt {
public:
  Token break_token;

  BreakStmt(Token break_token) : break_token(break_token) {}

  void accept(Visitor *visitor) { visitor->visit_break_stmt(this); }
};