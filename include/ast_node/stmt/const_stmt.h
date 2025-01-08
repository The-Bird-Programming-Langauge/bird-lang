#pragma once

#include <string>
#include <memory>
#include "stmt.h"
#include "../../token.h"
#include "visitors/visitor.h"

// forward declaration
class Expr;

/*
 * Const Declaration statement AST Node that represents variable declarations
 * ex:
 * const x: int = 4;
 */
class ConstStmt : public Stmt
{
public:
    Token identifier;
    std::optional<Token> type_token;
    bool type_is_literal;
    std::unique_ptr<Expr> value;

    ConstStmt(Token identifier, std::optional<Token> type_token, bool type_is_literal, std::unique_ptr<Expr> value)
    {
        this->identifier = identifier;
        this->type_token = type_token;
        this->type_is_literal = type_is_literal;
        this->value = std::move(value);
    }

    ConstStmt(Token identifier, std::optional<Token> type_token, bool type_is_literal, Expr *value)
        : identifier(identifier), type_token(type_token), type_is_literal(type_is_literal), value(value) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_const_stmt(this);
    }
};
