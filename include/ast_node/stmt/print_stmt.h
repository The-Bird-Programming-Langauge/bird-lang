#pragma once

#include <string>
#include <memory>
#include <vector>

#include "stmt.h"
#include "../../token.h"
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
    std::vector<std::unique_ptr<Expr>> args;

    PrintStmt(std::vector<std::unique_ptr<Expr>> args)
    {
        this->args = std::move(args);
    }

    PrintStmt(std::vector<Expr *> args)
    {
        this->args.reserve(args.size());
        for (Expr *arg : args)
        {
            this->args.push_back(std::unique_ptr<Expr>(arg));
        }
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_print_stmt(this);
    }
};
