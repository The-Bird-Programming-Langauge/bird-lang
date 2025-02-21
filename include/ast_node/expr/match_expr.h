
#pragma once
#include <vector>
#include <memory>
#include <optional>
#include "visitors/visitor.h"
#include "expr.h"

// forward declaration
class Visitor;

/*
 * Interface:
 * AST node representing a function call:
 *
 * double(3);
 */
class MatchExpr : public Expr
{
public:
    std::unique_ptr<Expr> expr;
    std::optional<std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>>> arms;
    std::unique_ptr<Expr> else_arm;

    MatchExpr(
        std::unique_ptr<Expr> expr,
        std::optional<std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>>> arms,
        std::unique_ptr<Expr> else_arm)
        : expr(std::move(expr)), arms(std::move(arms)), else_arm(std::move(else_arm)) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_match_expr(this);
    }
};
