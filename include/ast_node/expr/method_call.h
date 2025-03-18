#pragma once

#include "../../visitors/visitor.h"
#include "call.h"
#include "direct_member_access.h"
#include "expr.h"
#include <memory>

// forward declaration
class Visitor;
class Expr;

/*
 * Interface:
 * AST node representing a method call:
 *
 * struct Foo {
 * name: str;
 * fn say_name() {
 *   print self.name;
 * }
 *}
 *
 * const foo = Foo {
 *  name = "cole"
 *};
 * foo.say_name();
 */
class MethodCall : public DirectMemberAccess {
public:
  std::vector<std::shared_ptr<Expr>> args;
  std::vector<std::shared_ptr<ParseType::Type>> type_args;
  MethodCall(DirectMemberAccess *direct_member_access,
             std::vector<std::shared_ptr<Expr>> args,
             std::vector<std::shared_ptr<ParseType::Type>> type_args = {})
      : DirectMemberAccess(direct_member_access->accessable,
                           direct_member_access->identifier),
        args(args), type_args(std::move(type_args)) {};

  void accept(Visitor *visitor) { visitor->visit_method_call(this); }
};
