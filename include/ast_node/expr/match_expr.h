
#pragma once
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "expr.h"
#include <memory>
#include <optional>
#include <vector>

// forward declaration
class Visitor;

/*
 * Interface:
 * AST node representing a match expression:
 *
 * match foo {
 *   1 => "one",
 *   2 => "two",
 *   else => "other"
 * }
 */
class MatchExpr : public Expr {
public:
  Token match_token;
  std::unique_ptr<Expr> expr;
  std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> arms;
  std::unique_ptr<Expr> else_arm; // should else be optional?

  MatchExpr(
      Token token, std::unique_ptr<Expr> expr,
      std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> arms,
      std::unique_ptr<Expr> else_arm)
      : match_token(token), expr(std::move(expr)), arms(std::move(arms)),
        else_arm(std::move(else_arm)) {}

  void accept(Visitor *visitor) { visitor->visit_match_expr(this); }
};
