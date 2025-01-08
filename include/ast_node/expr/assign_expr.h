#pragma once

#include <string>
#include <memory>
#include "expr.h"
#include "../../token.h"
#include "visitors/visitor.h"

/*
 * Assignment statement AST Node that represents variable assignments
 * ex:
 * x = 4;
 * x += 4;
 */
class AssignExpr : public Expr
{
public:
    Token identifier;
    Token assign_operator;
    std::unique_ptr<Expr> value;

    AssignExpr(Token identifier, Token assign_operator, std::unique_ptr<Expr> value)
    {
        this->identifier = identifier;
        this->assign_operator = assign_operator;
        this->value = std::move(value);
    }

    AssignExpr(Token identifier, Token assign_operator, Expr *value)
        : identifier(identifier), assign_operator(assign_operator), value(value) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_assign_expr(this);
    }
};
