
#pragma once
#include "../ast_node/index.h"
#include "../exceptions/bird_exception.h"
#include "../stack.h"
#include "visitor.h"
#include "visitor_adapter.h"
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

class NameDecorator : public VisitorAdapter {
  Stack<std::string> ns_stack;
  Stack<std::string> r_stack;
  std::unordered_map<std::string, bool> seen;
  bool array_init = false;

public:
  void decorate(std::vector<std::unique_ptr<Stmt>> *stmts) {
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }
  }

  std::string get_current_namespace_prefix() {
    std::string prefix;
    for (auto &pref : this->ns_stack.stack) {
      prefix += pref;
    }
    return prefix;
  }

  std::string get_current_scope_resolution_prefix() {
    std::string prefix;
    for (auto &pref : this->r_stack.stack) {
      prefix += pref;
    }
    return prefix;
  }

  void visit_print_stmt(PrintStmt *print_stmt) {
    for (auto &arg : print_stmt->args) {
      arg->accept(this);
    }
  }

  void visit_direct_member_access(DirectMemberAccess *dma) {
    dma->accessable->accept(this);
  }

  void visit_primary(Primary *primary) {
    if (primary->value.token_type == Token::Type::IDENTIFIER) {
      if (!this->seen[primary->value.lexeme] && !this->array_init) {
        primary->value.lexeme =
            get_current_scope_resolution_prefix() + primary->value.lexeme;
      } else {
        primary->value.lexeme =
            get_current_namespace_prefix() + primary->value.lexeme;
      }
    }
  }

  void visit_decl_stmt(DeclStmt *decl_stmt) {
    decl_stmt->identifier.lexeme =
        get_current_namespace_prefix() + decl_stmt->identifier.lexeme;

    this->seen[decl_stmt->identifier.lexeme] = true;

    if (decl_stmt->type.has_value()) {
      std::string type_name = decl_stmt->type.value()->get_token().lexeme;
      std::string resolved = type_name;

      if (!this->seen[get_current_namespace_prefix() + type_name]) {
        resolved = get_current_scope_resolution_prefix() + type_name;
      } else {
        resolved = get_current_namespace_prefix() + type_name;
      }

      if (this->seen[resolved] &&
          (decl_stmt->type.value()->tag == ParseType::USER_DEFINED ||
           decl_stmt->type.value()->tag == ParseType::ARRAY)) {
        auto old_token = decl_stmt->type.value()->get_token();
        auto new_token = Token(old_token.token_type, resolved,
                               old_token.line_num, old_token.char_num);
        decl_stmt->type.value()->set_token(new_token);
      }
    }

    decl_stmt->value->accept(this);
  }

  void visit_const_stmt(ConstStmt *const_stmt) {
    const_stmt->identifier.lexeme =
        get_current_namespace_prefix() + const_stmt->identifier.lexeme;

    this->seen[const_stmt->identifier.lexeme] = true;

    if (const_stmt->type.has_value()) {
      std::string type_name = const_stmt->type.value()->get_token().lexeme;
      std::string resolved = type_name;

      if (!this->seen[get_current_namespace_prefix() + type_name]) {
        resolved = get_current_scope_resolution_prefix() + type_name;
      } else {
        resolved = get_current_namespace_prefix() + type_name;
      }

      if (this->seen[resolved] &&
          (const_stmt->type.value()->tag == ParseType::USER_DEFINED ||
           const_stmt->type.value()->tag == ParseType::ARRAY)) {
        auto old_token = const_stmt->type.value()->get_token();
        auto new_token = Token(old_token.token_type, resolved,
                               old_token.line_num, old_token.char_num);
        const_stmt->type.value()->set_token(new_token);
      }
    }

    const_stmt->value->accept(this);
  }
  void visit_struct_decl(StructDecl *struct_decl) {
    std::string qualified =
        get_current_namespace_prefix() + struct_decl->identifier.lexeme;

    if (!this->seen[qualified]) {
      struct_decl->identifier.lexeme = qualified;
      this->seen[qualified] = true;
    }

    for (auto &method : struct_decl->fns) {
      method->accept(this);
    }
  }

  void visit_array_init(ArrayInit *array_init) {
    this->array_init = true;
    for (auto &element : array_init->elements) {
      element->accept(this);
    }
    this->array_init = false;
  }

  void visit_struct_initialization(StructInitialization *si) {
    if (!this->seen[get_current_namespace_prefix() + si->identifier.lexeme]) {
      si->identifier.lexeme =
          get_current_scope_resolution_prefix() + si->identifier.lexeme;
    } else {
      si->identifier.lexeme = get_current_namespace_prefix() +
                              get_current_scope_resolution_prefix() +
                              si->identifier.lexeme;
    }
  }

  void visit_call(Call *call) { call->callable->accept(this); }

  void visit_type_stmt(TypeStmt *type_stmt) {
    type_stmt->identifier.lexeme =
        get_current_namespace_prefix() + type_stmt->identifier.lexeme;
  }

  void visit_func(Func *func) {
    func->identifier.lexeme =
        get_current_namespace_prefix() + func->identifier.lexeme;

    for (auto &param : func->param_list) {
      if (param.second->tag == ParseType::USER_DEFINED) {
        auto old_token = param.second->get_token();
        auto new_name = get_current_namespace_prefix() + old_token.lexeme;
        auto new_token = Token(old_token.token_type, new_name,
                               old_token.line_num, old_token.char_num);
        param.second->set_token(new_token);
      }
    }

    if (func->return_type &&
        func->return_type->get()->tag == ParseType::USER_DEFINED) {
      auto old_token = func->return_type->get()->get_token();
      auto new_name = get_current_namespace_prefix() + old_token.lexeme;
      auto new_token = Token(old_token.token_type, new_name, old_token.line_num,
                             old_token.char_num);
      func->return_type->get()->set_token(new_token);
    }

    func->block->accept(this);
  }

  void visit_method(Method *method) {
    method->class_identifier.lexeme =
        get_current_namespace_prefix() + method->class_identifier.lexeme;
    for (auto param : method->param_list) {
      if (param.second->tag == ParseType::USER_DEFINED) {
        auto old_token = param.second->get_token();
        auto new_name = get_current_namespace_prefix() + old_token.lexeme;
        auto new_token = Token(old_token.token_type, new_name,
                               old_token.line_num, old_token.char_num);
        param.second->set_token(new_token);
      }
    }

    if (method->return_type &&
        method->return_type->get()->tag == ParseType::USER_DEFINED) {
      auto old_token = method->return_type->get()->get_token();
      auto new_name = get_current_namespace_prefix() + old_token.lexeme;
      auto new_token = Token(old_token.token_type, new_name, old_token.line_num,
                             old_token.char_num);
      method->return_type->get()->set_token(new_token);
    }

    method->block->accept(this);
  }

  void visit_assign_expr(AssignExpr *assign_expr) {
    if (!this->seen[get_current_namespace_prefix() +
                    assign_expr->identifier.lexeme]) {
      assign_expr->identifier.lexeme = get_current_scope_resolution_prefix() +
                                       assign_expr->identifier.lexeme;
      this->seen[get_current_scope_resolution_prefix() +
                 assign_expr->identifier.lexeme] = true;
    }

    assign_expr->value->accept(this);
  }

  void visit_namespace(NamespaceStmt *_namespace) {
    this->ns_stack.push(_namespace->identifier.lexeme + "::");
    for (auto &member : _namespace->members) {
      member->accept(this);
    }
    this->ns_stack.pop();
  }

  void visit_scope_resolution(ScopeResolutionExpr *scope_resolution) {
    std::string ns = scope_resolution->_namespace.lexeme + "::";

    if (r_stack.empty() || r_stack.peek() != ns) {
      r_stack.push(ns);
    }

    scope_resolution->identifier->accept(this);

    if (!r_stack.empty() && r_stack.peek() == ns) {
      r_stack.pop();
    }
  }
};
