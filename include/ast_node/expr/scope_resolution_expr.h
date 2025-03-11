#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "assign_expr.h"
#include "expr.h"
#include <iostream>
#include <memory>
#include <string>

/*
 * Scope Resolution AST Node that represents variable stuct member
 */
class ScopeResolutionExpr : public Expr {
public:
  Token _namespace;
  std::unique_ptr<Expr> identifier;

  ScopeResolutionExpr(Token _namespace, std::unique_ptr<Expr> identifier)
      : _namespace(_namespace), identifier(std::move(identifier)) {}

  void accept(Visitor *visitor) { visitor->visit_scope_resolution(this); }
};
