#pragma once

#include <string>
#include <memory>
#include <vector>

#include "stmt.h"
#include "../../token.h"
#include "visitors/visitor.h"

/*
 * Block statement AST Node that represents blocks and new scopes
 * ex:
 * var x = 3;
 * {
 *     var x = 4;
 * }
 *
 */
class Block : public Stmt
{
public:
    std::vector<std::unique_ptr<Stmt>> stmts;

    Block(std::vector<std::unique_ptr<Stmt>> stmts)
    {
        this->stmts = std::move(stmts);
    }

    Block(std::vector<Stmt *> stmts)
    {
        this->stmts.reserve(stmts.size());
        for (Stmt *stmt : stmts)
        {
            this->stmts.push_back(std::unique_ptr<Stmt>(stmt));
        }
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_block(this);
    }
};
