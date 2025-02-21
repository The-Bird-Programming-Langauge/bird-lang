#pragma once
#include "ast_node/index.h"
#include "exceptions/bird_exception.h"
#include "visitor_adapter.h"
#include <memory>
#include <set>
#include <vector>

class HoistVisitor : public VisitorAdapter {
  std::set<std::string> &struct_names;

public:
  HoistVisitor(std::set<std::string> &struct_names)
      : struct_names(struct_names) {}

  void hoist(std::vector<std::unique_ptr<Stmt>> *stmts) {
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }
  }

  void visit_type_stmt(TypeStmt *type_stmt) {
    auto type_name = type_stmt->type_token->get_token().lexeme;

    if (this->struct_names.find(type_stmt->type_token->get_token().lexeme) !=
        this->struct_names.end()) {
      return;
    }

    this->struct_names.insert(type_stmt->type_token->get_token().lexeme);
  }

  void visit_struct_decl(StructDecl *struct_decl) {
    if (this->struct_names.find(struct_decl->identifier.lexeme) !=
        this->struct_names.end()) {
      return;
    }

    this->struct_names.insert(struct_decl->identifier.lexeme);
  }

  void visit_array_init(ArrayInit *array_init) {}

  void visit_match_expr(MatchExpr *match_expr) {}
};