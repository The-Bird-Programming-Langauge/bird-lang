
#pragma once
#include "../ast_node/index.h"
#include "../exceptions/bird_exception.h"
#include "../stack.h"
#include "visitor.h"
#include "visitor_adapter.h"
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class NameDecorator : public VisitorAdapter {
  Stack<std::string> ns_stack;
  Stack<std::string> r_stack;
  std::unordered_map<std::string, bool> seen;

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

  void visit_primary(Primary *primary) {
    if (primary->value.token_type == Token::Type::IDENTIFIER) {
      auto identifier = primary->value.lexeme;

      if (identifier == "self")
        return;

      auto from_r = get_current_scope_resolution_prefix() + identifier;
      auto from_ns = get_current_namespace_prefix() + identifier;

      if (seen.count(from_r)) {
        primary->value.lexeme = from_r;
        return;
      }

      if (seen.count(from_ns)) {
        primary->value.lexeme = from_ns;
        return;
      }

      std::string prefix;
      std::string global;
      std::vector<std::string> popped;

      while (!ns_stack.stack.empty()) {
        auto top = ns_stack.pop();
        popped.push_back(top);
        prefix = get_current_namespace_prefix();

        if (seen.count(prefix + identifier)) {
          global = prefix + identifier;
          break;
        }
      }

      for (auto it = popped.rbegin(); it != popped.rend(); ++it) {
        ns_stack.push(*it);
      }

      if (!global.empty()) {
        primary->value.lexeme = global;
        return;
      }

      if (seen.count(identifier)) {
        return;
      }
    }
  }

  void visit_decl_stmt(DeclStmt *decl_stmt) {
    decl_stmt->identifier.lexeme =
        get_current_namespace_prefix() + decl_stmt->identifier.lexeme;

    this->seen[decl_stmt->identifier.lexeme] = true;

    if (decl_stmt->type.has_value()) {
      auto resolved = decl_stmt->type->get()->get_token().lexeme;
      auto prefix = get_current_namespace_prefix();
      std::string global;

      if (!this->seen[prefix + resolved]) {
        std::vector<std::string> popped_stack;
        while (!ns_stack.stack.empty()) {
          auto popped = ns_stack.pop();
          popped_stack.push_back(popped);
          prefix = get_current_namespace_prefix();

          if (this->seen[prefix + get_current_scope_resolution_prefix() +
                         resolved]) {
            global = prefix + resolved;
            break;
          }
        }

        for (auto it = popped_stack.rbegin(); it != popped_stack.rend(); ++it) {
          ns_stack.push(*it);
        }

        if (!global.empty()) {
          resolved = global;
        } else {
          resolved = get_current_scope_resolution_prefix() + resolved;
        }
      } else {
        resolved = prefix + resolved;
      }

      if (this->seen[resolved] &&
          (decl_stmt->type.value()->tag == ParseType::USER_DEFINED ||
           decl_stmt->type.value()->tag == ParseType::ARRAY)) {
        auto previous_token = decl_stmt->type.value()->get_token();
        auto new_token =
            Token(previous_token.token_type, resolved, previous_token.line_num,
                  previous_token.char_num);
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
      auto resolved = const_stmt->type->get()->get_token().lexeme;
      auto prefix = get_current_namespace_prefix();
      std::string global;

      if (!this->seen[prefix + get_current_scope_resolution_prefix() +
                      resolved]) {
        std::vector<std::string> popped_stack;
        while (!ns_stack.stack.empty()) {
          auto popped = ns_stack.pop();
          popped_stack.push_back(popped);
          prefix = get_current_namespace_prefix();

          if (this->seen[prefix + get_current_scope_resolution_prefix() +
                         resolved]) {
            global = prefix + resolved;
            break;
          }
        }

        for (auto it = popped_stack.rbegin(); it != popped_stack.rend(); ++it) {
          ns_stack.push(*it);
        }

        if (!global.empty()) {
          resolved = global;
        } else {
          resolved = get_current_scope_resolution_prefix() + resolved;
        }
      } else {
        resolved = prefix + resolved;
      }

      if (this->seen[resolved] &&
          (const_stmt->type.value()->tag == ParseType::USER_DEFINED ||
           const_stmt->type.value()->tag == ParseType::ARRAY)) {
        auto previous_token = const_stmt->type.value()->get_token();
        auto new_token =
            Token(previous_token.token_type, resolved, previous_token.line_num,
                  previous_token.char_num);
        const_stmt->type.value()->set_token(new_token);
      }
    }

    const_stmt->value->accept(this);
  }

  void visit_struct_decl(StructDecl *struct_decl) {
    auto identifier = struct_decl->identifier.lexeme;
    auto prefixed_identifier = get_current_namespace_prefix() + identifier;

    if (!this->seen[prefixed_identifier]) {
      struct_decl->identifier.lexeme = prefixed_identifier;
      this->seen[prefixed_identifier] = true;
    }

    for (auto &method : struct_decl->fns) {
      method->accept(this);
    }
  }

  void visit_struct_initialization(StructInitialization *si) {
    auto resolved = si->identifier.lexeme;
    auto prefix = get_current_namespace_prefix();
    std::string global;

    if (!this->seen[prefix + get_current_scope_resolution_prefix() +
                    resolved]) {
      std::vector<std::string> popped_stack;
      while (!ns_stack.stack.empty()) {
        auto popped = ns_stack.pop();
        popped_stack.push_back(popped);
        prefix = get_current_namespace_prefix();

        if (this->seen[prefix + resolved]) {
          global = prefix + resolved;
          break;
        }
      }

      for (auto it = popped_stack.rbegin(); it != popped_stack.rend(); ++it) {
        ns_stack.push(*it);
      }

      if (!global.empty()) {
        si->identifier.lexeme = global;
      } else {
        si->identifier.lexeme =
            get_current_scope_resolution_prefix() + resolved;
      }

    } else {
      si->identifier.lexeme =
          prefix + get_current_scope_resolution_prefix() + resolved;
    }

    for (auto &field : si->field_assignments) {
      field.second->accept(this);
    }
  }

  void visit_type_stmt(TypeStmt *type_stmt) {
    type_stmt->identifier.lexeme =
        get_current_namespace_prefix() + type_stmt->identifier.lexeme;
  }

  void visit_func(Func *func) {
    func->identifier.lexeme =
        get_current_namespace_prefix() + func->identifier.lexeme;

    for (auto &param : func->param_list) {
      this->seen[param.second->get_token().lexeme] = true;

      if (param.second->tag == ParseType::USER_DEFINED) {
        auto previous_token = param.second->get_token();
        auto new_name = get_current_namespace_prefix() + previous_token.lexeme;
        auto new_token =
            Token(previous_token.token_type, new_name, previous_token.line_num,
                  previous_token.char_num);
        param.second->set_token(new_token);
      }
    }

    if (func->return_type &&
        (func->return_type.value()->tag == ParseType::USER_DEFINED ||
         func->return_type.value()->tag == ParseType::ARRAY)) {
      auto previous_token = func->return_type->get()->get_token();
      auto resolved = previous_token.lexeme;
      auto prefix = get_current_namespace_prefix();
      std::string global;

      if (!this->seen[prefix + resolved]) {
        std::vector<std::string> popped_stack;
        while (!ns_stack.stack.empty()) {
          auto popped = ns_stack.pop();
          popped_stack.push_back(popped);
          prefix = get_current_namespace_prefix();

          if (this->seen[prefix + resolved]) {
            global = prefix + resolved;
            break;
          }
        }

        for (auto it = popped_stack.rbegin(); it != popped_stack.rend(); ++it) {
          ns_stack.push(*it);
        }

        if (!global.empty()) {
          resolved = global;
        } else {
          resolved = get_current_scope_resolution_prefix() + resolved;
        }
      } else {
        resolved = prefix + resolved;
      }

      if (this->seen[resolved] &&
          (func->return_type->get()->tag == ParseType::USER_DEFINED ||
           func->return_type->get()->tag == ParseType::ARRAY)) {
        auto new_token =
            Token(previous_token.token_type, resolved, previous_token.line_num,
                  previous_token.char_num);
        func->return_type->get()->set_token(new_token);
      }
    }

    this->seen[func->identifier.lexeme] = true;
    func->block->accept(this);
  }

  void visit_method(Method *method) {
    method->class_identifier.lexeme =
        get_current_namespace_prefix() + method->class_identifier.lexeme;

    for (auto &param : method->param_list) {
      if (param.first.lexeme != "self") {
        this->seen[param.first.lexeme] = true;
      }

      if (param.second->tag == ParseType::USER_DEFINED) {
        auto previous_token = param.second->get_token();
        auto prefixed_name =
            get_current_namespace_prefix() + previous_token.lexeme;
        auto new_token =
            Token(previous_token.token_type, prefixed_name,
                  previous_token.line_num, previous_token.char_num);
        param.second->set_token(new_token);
      }
    }

    if (method->return_type &&
        (method->return_type->get()->tag == ParseType::USER_DEFINED ||
         method->return_type->get()->tag == ParseType::ARRAY)) {
      auto previous_token = method->return_type->get()->get_token();
      auto resolved = previous_token.lexeme;

      auto prefix = get_current_namespace_prefix();
      std::string global;

      if (!this->seen[prefix + resolved]) {
        std::vector<std::string> popped_stack;
        while (!ns_stack.stack.empty()) {
          auto popped = ns_stack.pop();
          popped_stack.push_back(popped);
          prefix = get_current_namespace_prefix();

          if (this->seen[prefix + get_current_scope_resolution_prefix() +
                         resolved]) {
            global = prefix + resolved;
            break;
          }
        }

        for (auto it = popped_stack.rbegin(); it != popped_stack.rend(); ++it) {
          ns_stack.push(*it);
        }

        if (!global.empty()) {
          resolved = global;
        } else {
          resolved = get_current_scope_resolution_prefix() + resolved;
        }
      } else {
        resolved = prefix + resolved;
      }

      if (this->seen[resolved] &&
          (method->return_type->get()->tag == ParseType::USER_DEFINED ||
           method->return_type->get()->tag == ParseType::ARRAY)) {
        auto new_token =
            Token(previous_token.token_type, resolved, previous_token.line_num,
                  previous_token.char_num);
        method->return_type->get()->set_token(new_token);
      }
    }

    this->seen[method->identifier.lexeme] = true;
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
    auto ns = scope_resolution->_namespace.lexeme + "::";

    if (r_stack.empty() || r_stack.peek() != ns) {
      r_stack.push(ns);
    }

    scope_resolution->identifier->accept(this);

    if (!r_stack.empty() && r_stack.peek() == ns) {
      r_stack.pop();
    }
  }
};
