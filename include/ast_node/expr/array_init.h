#pragma once

#include "../../visitors/visitor.h"
#include "expr.h"
#include <memory>
#include <vector>

/*
 * Primary class AST node representing array primitives
 * ex: [1,2,3]
 */
class ArrayInit : public Expr {
public:
  // TODO: does this need to be shared? I believe they can be unique
  std::vector<std::shared_ptr<Expr>> elements;

  ArrayInit(std::vector<std::shared_ptr<Expr>> elements)
      : elements(std::move(elements)) {}

  void accept(Visitor *visitor) { return visitor->visit_array_init(this); }
};
