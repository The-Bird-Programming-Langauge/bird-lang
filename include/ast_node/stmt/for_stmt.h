#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"
#include <memory>
#include <optional>

// forward declaration
class Expr;

/*
 * for statement AST Node
 * ex:
 *  for Stmt | Expr ";" Expr ";" Stmt {
 *      Stmt ";"
 *  }
 */
class ForStmt : public Stmt {
public:
  Token for_token;
  std::optional<std::unique_ptr<Stmt>> initializer;
  std::optional<std::unique_ptr<Expr>> condition;
  std::optional<std::unique_ptr<Expr>> increment;
  std::unique_ptr<Stmt> body;

  ForStmt(Token for_token, std::optional<std::unique_ptr<Stmt>> initializer,
          std::optional<std::unique_ptr<Expr>> condition,
          std::optional<std::unique_ptr<Expr>> increment,
          std::unique_ptr<Stmt> body)
      : for_token(for_token), initializer(std::move(initializer)),
        condition(std::move(condition)), increment(std::move(increment)),
        body(std::move(body)) {}

  void accept(Visitor *visitor) { visitor->visit_for_stmt(this); }
};