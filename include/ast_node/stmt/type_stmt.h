#pragma once

#include <string>
#include <memory>
#include <optional>

#include "stmt.h"
#include "token.h"
#include "visitors/visitor.h"

// forward declaration
class Expr;

/*
 * Type statement AST Node that represents type declarations
 * ex:
 * type x = int;
 */
class TypeStmt : public Stmt
{
public:
    Token identifier;
    Token type_token; // TODO: Create an ast node class for type tokens to categorize type literals, type identifiers, and type blocks. This class will be like stmt or expr.
    bool type_is_literal;

    TypeStmt(Token identifier, Token type_token, bool type_is_literal)
        : identifier(identifier),
          type_token(type_token),
          type_is_literal(type_is_literal) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_type_stmt(this);
    }
};