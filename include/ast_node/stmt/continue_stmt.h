#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"

/*
 * Continue statement class AST node that
 * represents loop continue
 * ex:
 * while (true) {
 *     continue;
 * }
 */
class ContinueStmt : public Stmt {
public:
  Token continue_token;

  ContinueStmt(Token continue_token) : continue_token(continue_token) {}

  void accept(Visitor *visitor) { visitor->visit_continue_stmt(this); }
};