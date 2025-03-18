#pragma once
#include "../ast_node/index.h"
#include "visitor_adapter.h"
#include <iostream>

/*
 * This visitor is responsible for removing generic polymorphic funcitons into
 * concrete implementations
 * Read more here: https://en.wikipedia.org/wiki/Monomorphization
 */

class Monomorphizer : public VisitorAdapter {

public:
  Monomorphizer() {};
  void monomorphize(std::vector<std::unique_ptr<Stmt>> *stmts) {
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }
  }

  void visit_func(Func *func) override {
    if (func->generic_identifiers.empty()) {
      return;
    }

    std::cout << "found generic funcdtion " << func->identifier.lexeme
              << std::endl;
  }

  void visit_method(Method *method) override {
    if (method->generic_identifiers.empty()) {
      return;
    }
  }

  void visit_call(Call *call) override {
    if (call->type_args.empty()) {
      std::cout << "empty type args" << std::endl;
      return;
    }

    std::cout << "found generic call" << std::endl;
  }

  void visit_method_call(MethodCall *method_call) override {
    if (method_call->type_args.empty()) {
      std::cout << "empty type args" << std::endl;
      return;
    }
    std::cout << "fond generic method call" << std::endl;
  }
};
