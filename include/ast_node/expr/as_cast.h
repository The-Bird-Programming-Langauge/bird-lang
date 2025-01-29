#pragma once

#include <memory>

#include "token.h"
#include "visitors/visitor.h"
#include "expr.h"

/*
 *
 * AST Node representing type casts
 * ex:
 *
 * type number = int;
 * var foo: number = 5 as number;
 *
 */
class AsCast : public Expr
{
public:
    std::unique_ptr<Expr> expr;
    Token type;

    AsCast(std::unique_ptr<Expr> expr, Token type) : expr(std::move(expr)), type(type) {}

    void accept(Visitor *visitor)
    {
        return visitor->visit_as_cast(this);
    }
};
