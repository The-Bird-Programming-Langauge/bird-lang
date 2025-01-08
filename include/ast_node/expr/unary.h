#pragma once

#include <memory>
#include "../../token.h"
#include "visitors/visitor.h"
#include "expr.h"

/*
 * Unary class AST node representing unary operations
 * ex:
 * -1
 * !true
 */
class Unary : public Expr
{
public:
    Token op; // must be of type -
    std::unique_ptr<Expr> expr;

    Unary(Token op, std::unique_ptr<Expr> expr)
    {
        this->op = op;
        this->expr = std::move(expr);
    }

    Unary(Token op, Expr *expr)
        : op(op), expr(expr) {}

    void accept(Visitor *visitor)
    {
        return visitor->visit_unary(this);
    }
};
