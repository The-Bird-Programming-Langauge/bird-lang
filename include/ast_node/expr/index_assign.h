#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "expr.h"
#include <memory>
#include <string>

/*
 * AST Node that represents array[index] assignments
 * ex:
 * arr[0] = 1;
 */
class IndexAssign : public Expr {
public:
  std::unique_ptr<Subscript> lhs;
  std::unique_ptr<Expr> rhs;
  Token op;

  IndexAssign(std::unique_ptr<Subscript> lhs, std::unique_ptr<Expr> rhs,
              Token op)
      : lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {}

  void accept(Visitor *visitor) { visitor->visit_index_assign(this); }
};
