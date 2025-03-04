#pragma once

#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "func.h"
#include "stmt.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

// forward declaration
class Visitor;

/*
 * Method definition AST Node
 *
 * struct Foo {
 *  fn foobar(foo: int, bar: int) -> int {
 *     statement;
 *     statement;
 *     statement;
 *  }
 *}
 */
class Method : public Func {
  Token class_identifier;

public:
  Method(Token class_identifier, Func *func)
      : Func(func), class_identifier(class_identifier) {}

  void accept(Visitor *visitor) { visitor->visit_method(this); }
};
