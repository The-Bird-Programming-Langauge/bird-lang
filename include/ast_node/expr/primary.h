#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "../capturable.h"
#include "expr.h"

/*
 * Primary class AST node representing primary values
 * ex:
 * - numbers: 1, 23
 * - identifiers: foo
 */
class Primary : public Expr, Captureable {
  bool captured = false;

public:
  Token value;

  Primary(Token value) : value(value) {}

  void accept(Visitor *visitor) { return visitor->visit_primary(this); }
  bool get_captured() { return captured; }
  void set_captured(bool captured) { this->captured = captured; }
};
