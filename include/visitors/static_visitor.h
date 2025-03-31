#pragma once
#include "../ast_node/index.h"
#include "visitor_adapter.h"
#include <cmath>
#include <memory>
#include <vector>

// TOOD: check for strings in function call
class StaticVisitor : public VisitorAdapter {
public:
  std::vector<std::string> &static_strings;

  StaticVisitor(std::vector<std::string> &static_strings)
      : static_strings(static_strings) {}
  ~StaticVisitor() {}

  void static_pass(std::vector<std::unique_ptr<Stmt>> *stmts) {
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }
  }

  void visit_assign_expr(AssignExpr *assign_expr) {
    switch (assign_expr->assign_operator.token_type) {
    case Token::Type::EQUAL: {
      assign_expr->value->accept(this);
      break;
    }
    default:
      break;
    }
  }

  void visit_primary(Primary *primary) {
    switch (primary->value.token_type) {
    case Token::Type::STR_LITERAL: {
      this->static_strings.push_back(primary->value.lexeme);
      break;
    }
    default:
      break;
    }
  }
};