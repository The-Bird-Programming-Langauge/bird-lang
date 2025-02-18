#pragma once

#include <string>
#include <memory>
#include "stmt.h"
#include "token.h"
#include "visitors/visitor.h"
#include <optional>

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
    std::optional<std::shared_ptr<ParseType::Type>> type;
    std::unique_ptr<Expr> value;

    ConstStmt(Token identifier, std::optional<std::shared_ptr<ParseType::Type>> type, std::unique_ptr<Expr> value)
        : identifier(identifier),
          type(std::move(type)),
          value(std::move(value))
    {
    }

    void accept(Visitor *visitor)
    {
        visitor->visit_const_stmt(this);
    }
};
