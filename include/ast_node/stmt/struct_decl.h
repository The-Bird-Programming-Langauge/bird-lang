#pragma once

#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <utility>

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
class StructDecl : public Stmt
{
public:
    Token identifier;
    std::vector<std::pair<std::string, Token>> fields;

    StructDecl(Token identifier, std::vector<std::pair<std::string, Token>> fields)
        : identifier(identifier),
          fields(fields) {}

    void accept(Visitor *visitor)
    {
        visitor->visit_struct_decl(this);
    }
};