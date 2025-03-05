#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "call.h"
#include "expr.h"
#include <iostream>
#include <memory>
#include <vector>

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
class MethodCall : public Call {
public:
  std::unique_ptr<Expr> instance;

  MethodCall(std::unique_ptr<Expr> instance, Call *call)
      : Call(call), instance(std::move(instance)) {};

  void accept(Visitor *visitor) { visitor->visit_method_call(this); }
};
