#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ast_node/node.h"
#include "ast_node/expr/expr.h"
#include "token.h"
#include "visitors/visitor.h"

// forward declaration
class Visitor;
class Expr;

/*
 * Interface:
 * AST node representing a subscript operation:
 *
 * const foo = "hello";
 * print foo[0]; // prints 'h'
 */
class Subscript : public Expr
{
public:
    std::unique_ptr<Expr> subscriptable;
    std::unique_ptr<Expr> index;
    Token subscript_token;

    Subscript(std::unique_ptr<Expr> subscriptable, std::unique_ptr<Expr> index, Token subscript_token)
        : subscriptable(std::move(subscriptable)),
          index(std::move(index)), subscript_token(subscript_token) {};

    void accept(Visitor *visitor)
    {
        visitor->visit_subscript(this);
    }
};
