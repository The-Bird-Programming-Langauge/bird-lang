#pragma once

#include "../node.h"
#include <any>
#include <string>

// forward declaration
class Visitor;

/*
 * Interface:
 * parent AST Node from which all expressions are derived
 */
class Expr : public Node {
public:
  virtual ~Expr() = default;
  virtual void accept(Visitor *visitor) = 0;
};
