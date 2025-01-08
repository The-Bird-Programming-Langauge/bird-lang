#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ast_node/node.h"
#include "../../token.h"
#include "visitors/visitor.h"

// forward declaration
class Visitor;
class Expr;

/*
 * Interface:
 * AST node representing a function call:
 *
 * double(3);
 */
class Call : public Expr
{
public:
    Token identifier;
    std::vector<std::shared_ptr<Expr>> args;

    Call(Token identifier, std::vector<std::shared_ptr<Expr>> args)
        : identifier(identifier), args(std::move(args)) {};

    Call(Token identifier, std::vector<Expr *> args)
        : identifier(identifier)
    {
        this->args.reserve(args.size());
        for (Expr *arg : args)
        {
            this->args.push_back(std::shared_ptr<Expr>(arg));
        }
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_call(this);
    }
};
