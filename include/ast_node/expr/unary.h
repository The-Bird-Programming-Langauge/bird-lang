#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "expr.h"
#include <memory>

/*
 * Unary class AST node representing unary operations
 * ex:
 * -1
 * !true
 */
class Unary : public Expr {
public:
  Token op;
  std::unique_ptr<Expr> expr;

  Unary(Token op, std::unique_ptr<Expr> expr) : op(op), expr(std::move(expr)) {}

  void accept(Visitor *visitor) { return visitor->visit_unary(this); }
};
