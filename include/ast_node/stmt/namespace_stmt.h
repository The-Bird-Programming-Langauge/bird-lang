#pragma once

#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <map>

// forward declaration
class Visitor;

/*
 * Namespace definition AST Node
 */
class NamespaceStmt : public Stmt
{
public:
  Token identifier;
  std::vector<std::unique_ptr<Stmt>> members;

  NamespaceStmt(Token identifier, std::vector<std::unique_ptr<Stmt>> members)
      : identifier(identifier), members(std::move(members)) {}

  void accept(Visitor *visitor) { visitor->visit_namespace(this); }
};
