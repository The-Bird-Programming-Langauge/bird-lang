#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "expr.h"
#include <memory>
#include <vector>

// forward declaration
class Visitor;
class Expr;

/*
 * Interface:
 * AST node representing a function call:
 *
 * double(3);
 */
class Call : public Expr {
public:
  std::unique_ptr<Expr> callable;
  std::vector<std::shared_ptr<Expr>> args;
  Token call_identifier;

  Call(std::unique_ptr<Expr> callable, std::vector<std::shared_ptr<Expr>> args,
       Token call_identifier)
      : callable(std::move(callable)), args(std::move(args)),
        call_identifier(call_identifier) {};

  void accept(Visitor *visitor) { visitor->visit_call(this); }
};
