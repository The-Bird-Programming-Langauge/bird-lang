
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

  bool is_primitive(const std::string &name) {
    return name == "int" || name == "float" || name == "bool" ||
           name == "str" || name == "void";
  }

  std::string resolve_identifier(std::string &identifier) {
    // check local namespace for a scoped variable
    auto from_local = get_current_namespace_prefix() + get_current_scope_resolution_prefix() + identifier;
    if (seen[from_local])
      return from_local;

    // check if it is fully qualified as-is
    auto from_r = get_current_scope_resolution_prefix() + identifier;
    if (seen[from_r])
      return from_r;

    // check local namespace 
    auto from_ns = get_current_namespace_prefix() + identifier;
    if (seen[from_ns])
      return from_ns;
    
    // if we make it here, it is declared outside of the namespace or not fully qualified
    std::vector<std::string> popped;
    std::string global;
    while (!ns_stack.empty()) {
      auto top = ns_stack.pop();
      popped.push_back(top);
      if (seen[get_current_namespace_prefix() + identifier]) {
        global = get_current_namespace_prefix() + identifier;
        break;
      }
    }
    // restore stack
    for (auto it = popped.rbegin(); it != popped.rend(); ++it) {
      ns_stack.push(*it);
    }
    return global.empty() ? identifier : global;
  }
  
  void
  maybe_resolve_type(std::optional<std::shared_ptr<ParseType::Type>> &type) {
    if (type && (type.value()->tag == ParseType::USER_DEFINED ||
                 type.value()->tag == ParseType::ARRAY)) {
      auto identifier = type.value()->get_token().lexeme;
      auto resolved = resolve_identifier(identifier);
      if (seen[resolved]) {
        set_type_token(type.value(), resolved);
      }
    }
  }

  void decorate_param_list(
      std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>> &params) {
    for (auto &param : params) {
      if (param.first.lexeme != "self")
        seen[param.first.lexeme] = true;

      if (param.second->tag == ParseType::USER_DEFINED ||
          param.second->tag == ParseType::ARRAY) {
        auto identifier = param.second->get_token().lexeme;
        // checks for array primitive
        if (!is_primitive(identifier)) {
          auto resolved = resolve_identifier(identifier);
          set_type_token(param.second, resolved);
        }
      }
    }
  }

  void set_type_token(std::shared_ptr<ParseType::Type> &type,
                      const std::string &resolved) {
    auto token = type->get_token();
    Token new_token(token.token_type, resolved, token.line_num, token.char_num);
    type->set_token(new_token);
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
      auto resolved = resolve_identifier(primary->value.lexeme);
      primary->value.lexeme = resolved;
    }
  }

  void visit_decl_stmt(DeclStmt *decl_stmt) {
    decl_stmt->identifier.lexeme =
        get_current_namespace_prefix() + decl_stmt->identifier.lexeme;

    maybe_resolve_type(decl_stmt->type);

    seen[decl_stmt->identifier.lexeme] = true;
    decl_stmt->value->accept(this);
  }

  void visit_const_stmt(ConstStmt *const_stmt) {
    const_stmt->identifier.lexeme =
        get_current_namespace_prefix() + const_stmt->identifier.lexeme;

    maybe_resolve_type(const_stmt->type);

    seen[const_stmt->identifier.lexeme] = true;
    const_stmt->value->accept(this);
  }

  void visit_struct_decl(StructDecl *struct_decl) {
    auto from_ns =
        get_current_namespace_prefix() + struct_decl->identifier.lexeme;

    if (!this->seen[from_ns]) {
      struct_decl->identifier.lexeme = from_ns;
      this->seen[from_ns] = true;
    }

    for (auto &method : struct_decl->fns) {
      method->accept(this);
    }
  }

  void visit_struct_initialization(StructInitialization *si) {
    si->identifier.lexeme = resolve_identifier(si->identifier.lexeme);

    for (auto &field : si->field_assignments) {
      field.second->accept(this);
    }
  }

  void visit_type_stmt(TypeStmt *type_stmt) {
    type_stmt->identifier.lexeme =
        get_current_namespace_prefix() + type_stmt->identifier.lexeme;

    seen[type_stmt->identifier.lexeme] = true;
  }

  void visit_func(Func *func) {
    func->identifier.lexeme =
        get_current_namespace_prefix() + func->identifier.lexeme;

    decorate_param_list(func->param_list);
    maybe_resolve_type(func->return_type);

    seen[func->identifier.lexeme] = true;
    func->block->accept(this);
  }

  void visit_method(Method *method) {
    method->class_identifier.lexeme =
        get_current_namespace_prefix() + method->class_identifier.lexeme;

    decorate_param_list(method->param_list);
    maybe_resolve_type(method->return_type);

    seen[method->identifier.lexeme] = true;
    method->block->accept(this);
  }

  void visit_assign_expr(AssignExpr *assign_expr) {
    assign_expr->identifier.lexeme =
        resolve_identifier(assign_expr->identifier.lexeme);

    seen[assign_expr->identifier.lexeme] = true;
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

    // need these checks for assignments, which might have a scope resolution on
    // lhs and rhs causing double push
    if (r_stack.empty() || r_stack.peek() != ns) {
      r_stack.push(ns);
    }

    scope_resolution->identifier->accept(this);

    if (!r_stack.empty() && r_stack.peek() == ns) {
      r_stack.pop();
    }
  }
};
