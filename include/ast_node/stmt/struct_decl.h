#pragma once

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"

// forward declaration
class Expr;

/*
 * Type statement AST Node that represents type declarations
 * ex:
 * type x = int;
 */
class StructDecl : public Stmt {
public:
  Token identifier;
  std::vector<std::pair<std::string, std::shared_ptr<ParseType::Type>>> fields;

  StructDecl(
      Token identifier,
      std::vector<std::pair<std::string, std::shared_ptr<ParseType::Type>>>
          fields)
      : identifier(identifier), fields(fields) {}

  void accept(Visitor *visitor) { visitor->visit_struct_decl(this); }
};