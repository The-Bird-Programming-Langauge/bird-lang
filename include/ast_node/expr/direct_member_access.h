#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "expr.h"
#include <memory>

// forward declaration
class Visitor;
class Expr;

/*
 * Interface:
 * AST node representing a direct member access operation:
 *
 * foo: Foo = Foo{ bar: 42 };
 * foo.bar;
 *
 */

class DirectMemberAccess : public Expr {
public:
  std::shared_ptr<Expr> accessable;
  Token identifier;

  DirectMemberAccess(std::unique_ptr<Expr> accessable, Token identifier)
      : accessable(std::move(accessable)), identifier(identifier) {};
  DirectMemberAccess(std::shared_ptr<Expr> accessable, Token identifier)
      : accessable(std::move(accessable)), identifier(identifier) {};

  void accept(Visitor *visitor) { visitor->visit_direct_member_access(this); }
};
