#pragma once

#include <string>
#include <memory>
#include "expr.h"
#include "token.h"
#include "visitors/visitor.h"
#include "assign_expr.h"

/*
 * Assignment statement AST Node that represents variable stuct member assignments
 * ex:
 * foo.bar = 42;
 */
class MemberAssign : public AssignExpr
{
public:
    std::unique_ptr<Expr> accessable;

    MemberAssign(std::unique_ptr<Expr> accessable, Token member_identifier, Token assign_operator, std::unique_ptr<Expr> value)
        : AssignExpr(member_identifier, assign_operator, std::move(value)), accessable(std::move(accessable)) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_member_assign(this);
    }
};
