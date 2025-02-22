#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"
#include <memory>
#include <optional>

// forward declaration
class Expr;

/*
 * Return statement class AST node that
 * represents what is returned from a function
 * ex:
 * fn double(x: int) -> int {
 *  return x * 2;
 * }
 */
class ReturnStmt : public Stmt {
public:
  Token return_token; // for error handling
  std::optional<std::unique_ptr<Expr>> expr;

  ReturnStmt(Token return_token, std::optional<std::unique_ptr<Expr>> expr)
      : return_token(return_token), expr(std::move(expr)) {}

  void accept(Visitor *visitor) { visitor->visit_return_stmt(this); }
};
