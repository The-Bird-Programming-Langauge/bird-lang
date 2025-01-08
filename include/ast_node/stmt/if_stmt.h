#pragma once

#include <memory>
#include <optional>
#include "stmt.h"
#include "visitors/visitor.h"

// forward declaration
class Expr;

/*
 * if statement AST Node
 * ex:
 * if "(" expression ")" "{" statement "}"
 * else if "{" statment "}"
 * else "{" statement "}"
 *
 * or
 *
 * ex:
 * if expression
 *       statement
 * else
 *       statment
 */
class IfStmt : public Stmt
{
public:
    Token if_token;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::optional<std::unique_ptr<Stmt>> else_branch;

    IfStmt(Token if_token,
           std::unique_ptr<Expr> condition,
           std::unique_ptr<Stmt> then_branch,
           std::optional<std::unique_ptr<Stmt>> else_branch)
        : if_token(if_token),
          condition(std::move(condition)),
          then_branch(std::move(then_branch))
    {
        if (else_branch.has_value())
        {
            this->else_branch = std::move(else_branch);
        }
        else
        {
            this->else_branch = std::nullopt;
        }
    }

    IfStmt(Token if_token, Expr *condition, Stmt *then_branch, std::optional<Stmt *> else_branch)
        : if_token(if_token), condition(condition), then_branch(then_branch)
    {
        if (else_branch.has_value())
        {
            this->else_branch = std::move(std::unique_ptr<Stmt>(else_branch.value()));
        }
        else
        {
            this->else_branch = std::nullopt;
        }
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_if_stmt(this);
    }
};
