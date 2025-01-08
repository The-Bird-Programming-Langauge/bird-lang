#pragma once

#include <any>
#include <string>
#include "ast_node/node.h"

// forward declaration
class Visitor;

/*
 * Interface:
 * parent AST Node from which all expressions are derived
 */
class Expr : public Node
{
public:
    virtual ~Expr() = default;
    virtual void accept(Visitor *visitor) = 0;
};
