#pragma once

#include <string>
#include <memory>
#include <vector>

#include "stmt.h"
#include "token.h"
#include "visitors/visitor.h"

// forward declaration
class Expr;

/*
 * Print statement AST Node
 * ex:
 * print 1
 */
class PrintStmt : public Stmt
{
public:
    std::vector<std::shared_ptr<Expr>> args;
    Token print_token;

    PrintStmt(std::vector<std::shared_ptr<Expr>> args, Token print_token) : args(std::move(args)), print_token(print_token) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_print_stmt(this);
    }
};
