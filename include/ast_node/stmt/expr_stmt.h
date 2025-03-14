#pragma once

#include "../../visitors/visitor.h"
#include "stmt.h"
#include <memory>

// forward declaration
class Expr;

/*
 * Expression statement class AST node that
 * represents expressions that end in ';'
 * ex:
 * (1 + 2);
 */
class ExprStmt : public Stmt {
public:
  std::unique_ptr<Expr> expr;

  ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}

  void accept(Visitor *visitor) { visitor->visit_expr_stmt(this); }
};
