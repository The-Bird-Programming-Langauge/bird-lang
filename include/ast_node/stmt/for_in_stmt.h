#pragma once

#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"
#include <memory>
#include <optional>

// forward declaration
class Expr;

/*
 * for-in statement AST Node
 * ex:
 *  for x in iter(arr) {
      print x;
 *  }
 */
class ForInStmt : public Stmt {
public:
  Token for_token;
  Token identifier;
  std::unique_ptr<Expr> iterable;
  std::unique_ptr<Stmt> body;

  ForInStmt(Token for_token, Token identifier, std::unique_ptr<Expr> iterable,
            std::unique_ptr<Stmt> body)
      : for_token(for_token), identifier(identifier),
        iterable(std::move(iterable)), body(std::move(body)) {}

  void accept(Visitor *visitor) { visitor->visit_for_in_stmt(this); }
};