#pragma once

#include "../../token.h"
#include "visitors/visitor.h"
#include "expr.h"

/*
 * Primary class AST node representing primary values
 * ex:
 * - numbers: 1, 23
 * - identifiers: foo
 */
class Primary : public Expr
{
public:
    Token value; // must be i32 literal
    Primary(Token value)
    {
        this->value = value;
    }

    void accept(Visitor *visitor)
    {
        return visitor->visit_primary(this);
    }
};
