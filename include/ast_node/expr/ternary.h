#pragma once

#include <memory>
#include "../../token.h"
#include "visitors/visitor.h"
#include "expr.h"

/*
 * Ternary class AST node representing ternary operations
 * ex:
 * ( x > y) ? print 1 : print 2;
 */
class Ternary : public Expr
{
public:
    std::unique_ptr<Expr> condition;
    Token ternary_token;
    std::unique_ptr<Expr> true_expr;
    std::unique_ptr<Expr> false_expr;

    Ternary(std::unique_ptr<Expr> condition,
            Token ternary_token,
            std::unique_ptr<Expr> true_expr,
            std::unique_ptr<Expr> false_expr)
        : condition(std::move(condition)),
          ternary_token(ternary_token),
          true_expr(std::move(true_expr)),
          false_expr(std::move(false_expr)) {}

    Ternary(Expr *condition,
            Token ternary_token,
            Expr *true_expr,
            Expr *false_expr)
        : condition(std::move(condition)),
          ternary_token(ternary_token),
          true_expr(std::move(true_expr)),
          false_expr(std::move(false_expr)) {}

    void accept(Visitor *visitor)
    {
        return visitor->visit_ternary(this);
    }
};
