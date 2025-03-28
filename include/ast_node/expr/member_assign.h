#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "assign_expr.h"
#include "expr.h"
#include <memory>
#include <string>

/*
 * Assignment statement AST Node that represents variable stuct member
 * assignments ex: foo.bar = 42;
 */
class MemberAssign : public AssignExpr {
public:
  std::shared_ptr<Expr> accessable;

  MemberAssign(std::shared_ptr<Expr> accessable, Token member_identifier,
               Token assign_operator, std::unique_ptr<Expr> value)
      : AssignExpr(member_identifier, assign_operator, std::move(value)),
        accessable(std::move(accessable)) {}

  void accept(Visitor *visitor) { visitor->visit_member_assign(this); }
};
