#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "expr.h"
#include <memory>
#include <string>

/*
 * Assignment statement AST Node that represents variable assignments
 * ex:
 * x = 4;
 * x += 4;
 */
class AssignExpr : public Expr {
public:
  Token identifier;
  Token assign_operator;
  std::unique_ptr<Expr> value;

  AssignExpr(Token identifier, Token assign_operator,
             std::unique_ptr<Expr> value)
      : identifier(identifier), assign_operator(assign_operator),
        value(std::move(value)) {}

  void accept(Visitor *visitor) { visitor->visit_assign_expr(this); }
};
