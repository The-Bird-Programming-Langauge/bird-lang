#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ast_node/node.h"
#include "token.h"
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
        : identifier(identifier),
          args(std::move(args)) {};

    void accept(Visitor *visitor)
    {
        visitor->visit_call(this);
    }
};
