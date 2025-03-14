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
  Token call_token;
  std::shared_ptr<Expr> callable;
  std::vector<std::shared_ptr<Expr>> args;

  Call(Token call_token, std::unique_ptr<Expr> callable,
       std::vector<std::shared_ptr<Expr>> args)
      : call_token(call_token), callable(std::move(callable)),
        args(std::move(args)) {};

  Call(Call *call)
      : call_token(call->call_token), callable(std::move(call->callable)),
        args(std::move(call->args)) {}

  void accept(Visitor *visitor) { visitor->visit_call(this); }
};
