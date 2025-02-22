#pragma once

#include "../../ast_node/node.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "expr.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

// forward declaration
class Visitor;
class Expr;

/*
 * Interface:
 * AST node representing a struct initialization
 *
 * foo: Foo = Foo{ bar: 42 };
 *
 */

class StructInitialization : public Expr {
public:
  Token identifier;
  std::vector<std::pair<std::string, std::unique_ptr<Expr>>> field_assignments;

  StructInitialization(
      Token identifier,
      std::vector<std::pair<std::string, std::unique_ptr<Expr>>>
          field_assignments)
      : identifier(identifier) {
    this->field_assignments = std::move(field_assignments);
    std::sort(this->field_assignments.begin(), this->field_assignments.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
  };

  void accept(Visitor *visitor) { visitor->visit_struct_initialization(this); }
};
