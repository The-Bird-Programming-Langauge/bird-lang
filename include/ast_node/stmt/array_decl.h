#pragma once

#include <string>
#include <memory>
#include <vector>
#include "stmt.h"
#include "token.h"
#include "visitors/visitor.h"

// forward declaration
class Visitor;
class Expr;

/*
 * var IDENTIFIER: TYPE_IDENTIFIER[] = [];
 */
class ArrayDecl : public Stmt
{
public:
    Token identifier;
    Token type_identifier;
    std::vector<std::shared_ptr<Expr>> elements;

    ArrayDecl(Token identifier,
              Token type_identifier,
              std::vector<std::shared_ptr<Expr>> elements)
        : identifier(identifier),
          type_identifier(type_identifier),
          elements(std::move(elements)) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_array_decl(this);
    }
};