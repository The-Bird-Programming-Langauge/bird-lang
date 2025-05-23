#pragma once

#include <memory>
#include <optional>
#include <string>

#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"

// forward declaration
class Expr;

/*
 * Declaration statement AST Node that represents variable declarations
 * ex:
 * var x = 4;
 */
class DeclStmt : public Stmt {
public:
  Token identifier;
  std::optional<std::shared_ptr<ParseType::Type>> type;
  std::unique_ptr<Expr> value;

  DeclStmt(Token identifier,
           std::optional<std::shared_ptr<ParseType::Type>> type,
           std::unique_ptr<Expr> value)
      : identifier(identifier), type(std::move(type)), value(std::move(value)) {
  }

  void accept(Visitor *visitor) { visitor->visit_decl_stmt(this); }
};
