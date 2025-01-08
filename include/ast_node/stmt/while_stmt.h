#pragma once

#include <memory>
#include "stmt.h"
#include "visitors/visitor.h"

// forward declaration
class Expr;

/*
 * while statement AST Node
 * ex:
 * while %expression% %statement%
 */
class WhileStmt : public Stmt
{
public:
    Token while_token;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> stmt;

    WhileStmt(Token while_token, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> stmt)
    {
        this->while_token = while_token;
        this->condition = std::move(condition);
        this->stmt = std::move(stmt);
    }

    WhileStmt(Token while_token, Expr *condition, Stmt *stmt)
        : while_token(while_token), condition(condition), stmt(stmt) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_while_stmt(this);
    }
};