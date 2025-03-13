#pragma once

#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

// forward declaration
class Visitor;

/*
 * Function definition AST Node
 *
 * fn foobar(foo: int, bar: int) -> int {
 *    statement;
 *    statement;
 *    statement;
 * }
 */
class Func : public Stmt {
public:
  Token identifier;
  std::optional<std::shared_ptr<ParseType::Type>> return_type;
  // the first item in the pair is an identifier, the second is a type
  std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>>
      param_list; // TODO: make this an actual type
  std::shared_ptr<Stmt> block;

  Func(Token identifier,
       std::optional<std::shared_ptr<ParseType::Type>> return_type,
       std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>>
           param_list,
       std::unique_ptr<Stmt> block)
      : identifier(identifier), return_type(return_type),
        param_list(param_list), block(std::move(block)) {}
  Func(Func *func)
      : identifier(func->identifier), return_type(func->return_type),
        param_list(func->param_list), block(func->block) {}

  void accept(Visitor *visitor) { visitor->visit_func(this); }
};
