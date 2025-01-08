#pragma once

#include <memory>
#include "stmt.h"
#include "visitors/visitor.h"

// forward declaration
class Expr;

/*
 * Expression statement class AST node that
 * represents expressions that end in ';'
 * ex:
 * (1 + 2);
 */
class ExprStmt : public Stmt
{
public:
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> expr)
    {
        this->expr = std::move(expr);
    }

    ExprStmt(Expr *expr)
        : expr(expr) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_expr_stmt(this);
    }
};
