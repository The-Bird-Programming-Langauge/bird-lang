#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"
#include <memory>
#include <optional>

// forward declaration
class Expr;

/*
 * if statement AST Node
 * ex:
 * if "(" expression ")" "{" statement "}"
 * else if "{" statment "}"
 * else "{" statement "}"
 *
 * or
 *
 * ex:
 * if expression
 *       statement
 * else
 *       statment
 */
class IfStmt : public Stmt {
public:
  Token if_token;
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> then_branch;
  std::optional<std::unique_ptr<Stmt>> else_branch;

  IfStmt(Token if_token, std::unique_ptr<Expr> condition,
         std::unique_ptr<Stmt> then_branch,
         std::optional<std::unique_ptr<Stmt>> else_branch)
      : if_token(if_token), condition(std::move(condition)),
        then_branch(std::move(then_branch)),
        else_branch(std::move(else_branch)) {}

  void accept(Visitor *visitor) { visitor->visit_if_stmt(this); }
};
