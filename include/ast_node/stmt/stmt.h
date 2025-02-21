#pragma once

#include "../node.h"

// forward declaration
class Visitor;

/*
 * Interface:
 * parent AST Node from which all statements are derived
 */
class Stmt : public Node {
public:
  virtual ~Stmt() = default;
  virtual void accept(Visitor *visitor) = 0;
};
