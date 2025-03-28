#pragma once
#include "../ast_node/index.h"
#include "../sym_table.h"
#include "visitor_adapter.h"
#include <iostream>
#include <set>
#include <unordered_map>

class ClosureFinder : public VisitorAdapter {
  bool in_lambda = false;

  Environment<Captureable *> environment; // stores the name
  unsigned int depth = 0;
  unsigned int captured_count = 0;
  std::unordered_map<std::string, unsigned int> function_capture_size;

public:
  ClosureFinder() { environment.push_env(); }

  std::unordered_map<std::string, unsigned int>
  find_captured(std::vector<std::unique_ptr<Stmt>> *stmts) {
    std::cout << "finding captured" << std::endl;
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }

    return this->function_capture_size;
  };

  void visit_decl_stmt(DeclStmt *decl_stmt) override {
    decl_stmt->value->accept(this);

    if (depth >= 1 && !in_lambda) {
      this->environment.declare(decl_stmt->identifier.lexeme, decl_stmt);
    }
  }

  void visit_const_stmt(ConstStmt *const_stmt) override {
    const_stmt->value->accept(this);

    if (depth >= 1 && !in_lambda) {
      this->environment.declare(const_stmt->identifier.lexeme, const_stmt);
    }
  }

  void set_captured(Captureable *stmt) {
    if (auto const_stmt = dynamic_cast<ConstStmt *>(stmt)) {
      std::cout << " found const" << std::endl;
      const_stmt->set_captured(true);
      return;
    }

    if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt)) {
      std::cout << "found decl" << std::endl;
      decl_stmt->set_captured(true);
      return;
    }
  }

  void visit_primary(Primary *primary) override {
    if (!in_lambda)
      return;

    if (primary->value.token_type != Token::IDENTIFIER)
      return;

    std::cout << "captured: " << primary->value.lexeme << std::endl;
    this->captured_count += 1;
    primary->set_captured(true);

    auto stmt = this->environment.get(primary->value.lexeme);
    this->set_captured(stmt);
  }

  void visit_lambda(Lambda *lambda) override {
    std::cout << "visiting lambda" << std::endl;
    bool old_in_lambda = in_lambda;
    in_lambda = true;
    depth += 1;
    lambda->block->accept(this);
    in_lambda = old_in_lambda;
  }

  void visit_func(Func *func) override {
    depth += 1;
    auto count = this->captured_count;
    func->block->accept(this);
    auto captured_size = this->captured_count - count;
    std::cout << "captured size: " << captured_size << std::endl;
    // we need to know how many variables, of what type, and in what order
    // assume int, we can find how many

    this->function_capture_size[func->identifier.lexeme] = captured_size;
  }
};