#pragma once

#include "stmt.h"
#include "visitors/visitor.h"

/*
 * Break statement class AST node that
 * represents loop breaks
 * ex:
 * while (true) {
 *     break;
 * }
 */
class BreakStmt : public Stmt
{
public:
    Token break_token;

    BreakStmt(Token break_token)
    {
        this->break_token = break_token;
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_break_stmt(this);
    }
};