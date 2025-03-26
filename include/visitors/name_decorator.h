#pragma once
#include "../ast_node/index.h"
#include "../exceptions/bird_exception.h"
#include "../stack.h"
#include "visitor.h"
#include "visitor_adapter.h"
#include <memory>
#include <set>
#include <vector>

class NameDecorator : public VisitorAdapter {
  Stack<std::string> stack;

public:
  void decorate(std::vector<std::unique_ptr<Stmt>> *stmts) {
    std::cout << "hello, decorate!" << std::endl;
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }
  }

  std::string get_current_namespace_prefix() const {
    std::string prefix;
    for (auto &pref : this->stack.stack) {
      prefix += pref + "::";
    }
    return prefix;
  }

  void visit_print_stmt(PrintStmt *print_stmt) {
    for (auto &arg : print_stmt->args) {
      arg->accept(this);
    }
  }

  void visit_primary(Primary *primary) {}

  void visit_decl_stmt(DeclStmt *decl_stmt) {
    std::cout << "decl before: " << decl_stmt->identifier.lexeme << std::endl;

    decl_stmt->identifier.lexeme =
        get_current_namespace_prefix() + decl_stmt->identifier.lexeme;
    decl_stmt->value->accept(this);

    std::cout << "decl after: " << decl_stmt->identifier.lexeme << std::endl;
  }

  void visit_const_stmt(ConstStmt *const_stmt) {
    const_stmt->identifier.lexeme =
        get_current_namespace_prefix() + const_stmt->identifier.lexeme;
    const_stmt->value->accept(this);
  }

  void visit_struct_decl(StructDecl *struct_decl) {
    struct_decl->identifier.lexeme =
        get_current_namespace_prefix() + struct_decl->identifier.lexeme;

    for (auto &method : struct_decl->fns) {
      method->accept(this);
    }
  }

  void visit_call(Call *call) {
    std::cout << "FDAFA\n\n" << call->call_token.lexeme << std::endl;
    call->callable->accept(this);

    for (auto &arg : call->args) {
      arg->accept(this);
    }
  }

  void visit_type_stmt(TypeStmt *type_stmt) {
    type_stmt->identifier.lexeme =
        get_current_namespace_prefix() + type_stmt->identifier.lexeme;
  }

  void visit_func(Func *func) {
    func->identifier.lexeme =
        get_current_namespace_prefix() + func->identifier.lexeme;
    func->block->accept(this);
  }

  void visit_namespace(NamespaceStmt *_namespace) {
    this->stack.push(_namespace->identifier.lexeme);

    for (auto &member : _namespace->members) {
      member->accept(this);
    }

    this->stack.pop();
  }

  void visit_scope_resolution(ScopeResolutionExpr *scope_resolution) {
    std::cout << "scope resolution: " << scope_resolution->_namespace.lexeme
              << std::endl;

    if (auto *primary =
            dynamic_cast<Primary *>(scope_resolution->identifier.get())) {
      if (primary->value.token_type == Token::Type::IDENTIFIER) {
        std::cout << "renaming identifier in scope resolution: "
                  << primary->value.lexeme << " -> ";
        primary->value.lexeme = (scope_resolution->_namespace.lexeme +
                                 "::" + primary->value.lexeme);
        std::cout << primary->value.lexeme << std::endl;
      }
    } else {
      this->stack.push(scope_resolution->_namespace.lexeme);
      scope_resolution->identifier->accept(this);
      this->stack.pop();
    }
  }
};