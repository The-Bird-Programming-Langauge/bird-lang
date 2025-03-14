#pragma once

#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "../stmt/stmt.h"
#include "expr.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

// forward declaration
class Visitor;

/*
 * Lambda definition AST Node
 *
 * const x: ()int = fn() -> int {return 3;}
 *
 */
class Lambda : public Expr {
public:
  Token fn_token;
  std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>> param_list;
  std::optional<std::shared_ptr<ParseType::Type>> return_type;
  std::shared_ptr<Stmt> block;

  Lambda(Token fn_token,
         std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>>
             param_list,
         std::optional<std::shared_ptr<ParseType::Type>> return_type,
         std::unique_ptr<Stmt> block)
      : fn_token(fn_token), param_list(param_list), return_type(return_type),
        block(std::move(block)) {}

  void accept(Visitor *visitor) { visitor->visit_lambda(this); }
};
