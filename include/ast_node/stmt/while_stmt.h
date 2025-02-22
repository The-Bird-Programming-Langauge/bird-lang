#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"
#include <memory>

// forward declaration
class Expr;

/*
 * while statement AST Node
 * ex:
 * while %expression% %statement%
 */
class WhileStmt : public Stmt {
public:
  Token while_token;
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> stmt;

  WhileStmt(Token while_token, std::unique_ptr<Expr> condition,
            std::unique_ptr<Stmt> stmt)
      : while_token(while_token), condition(std::move(condition)),
        stmt(std::move(stmt)) {}

  void accept(Visitor *visitor) { visitor->visit_while_stmt(this); }
};