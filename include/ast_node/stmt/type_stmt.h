#pragma once

#include <string>
#include <memory>
#include <optional>

#include "stmt.h"
#include "token.h"
#include "visitors/visitor.h"
#include "parse_type.h"

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
    std::shared_ptr<ParseType::Type> type_token;

    TypeStmt(Token identifier, std::shared_ptr<ParseType::Type> type_token)
        : identifier(identifier),
          type_token(std::move(type_token)) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_type_stmt(this);
    }
};