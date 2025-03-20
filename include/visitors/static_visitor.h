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

  void visit_decl_stmt(DeclStmt *decl_stmt) { decl_stmt->value->accept(this); }

  void visit_expr_stmt(ExprStmt *expr_stmt) { expr_stmt->expr->accept(this); }

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

  void visit_binary(Binary *binary) {
    binary->left->accept(this);
    binary->right->accept(this);
  }

  void visit_print_stmt(PrintStmt *print_stmt) {
    for (auto &arg : print_stmt->args) {
      arg->accept(this);
    }
  }

  void visit_struct_decl(StructDecl *struct_decl) {
    for (auto fn : struct_decl->fns) {
      fn->accept(this);
    }
  }

  void visit_struct_initialization(StructInitialization *struct_intialization) {
    for (auto &field_assignment : struct_intialization->field_assignments) {
      field_assignment.second->accept(this);
    }
  }

  void visit_block(Block *block) {
    for (auto &stmt : block->stmts) {
      stmt->accept(this);
    }
  }

  void visit_func(Func *func) { func->block->accept(this); }

  void visit_if_stmt(IfStmt *if_stmt) {
    if_stmt->then_branch->accept(this);
    if (if_stmt->else_branch.has_value()) {
      if_stmt->else_branch->get()->accept(this);
    }
  }

  void visit_return_stmt(ReturnStmt *return_stmt) {
    if (return_stmt->expr.has_value()) {
      return_stmt->expr->get()->accept(this);
    }
  }

  void visit_ternary(Ternary *ternary_expr) {
    ternary_expr->condition->accept(this);
    ternary_expr->true_expr->accept(this);
    ternary_expr->false_expr->accept(this);
  }

  void visit_member_assign(MemberAssign *member_assign) {
    member_assign->accessable->accept(this);
    member_assign->value->accept(this);
  }

  void visit_const_stmt(ConstStmt *const_stmt) {
    const_stmt->value->accept(this);
  }

  void visit_for_stmt(ForStmt *for_stmt) { for_stmt->body->accept(this); }

  void visit_while_stmt(WhileStmt *while_stmt) {
    while_stmt->stmt->accept(this);
  }

  void visit_as_cast(AsCast *as_cast) { as_cast->expr->accept(this); }

  void visit_array_init(ArrayInit *array_init) {
    for (auto &element : array_init->elements) {
      element->accept(this);
    }
  }

  void visit_index_assign(IndexAssign *index_assign) {
    index_assign->lhs->subscriptable->accept(this);
    index_assign->rhs->accept(this);
  }

  void visit_match_expr(MatchExpr *match_expr) {
    match_expr->expr->accept(this);
    for (auto &arm : match_expr->arms) {
      arm.first->accept(this);
      arm.second->accept(this);
    }

    match_expr->else_arm->accept(this);
  }

  void visit_method(Method *method) { method->block->accept(this); }

  void visit_call(Call *call) {
    for (auto arg : call->args) {
      arg->accept(this);
    }
  }

  void visit_direct_member_access(DirectMemberAccess *access) {
    access->accessable->accept(this);
  }

  void visit_method_call(MethodCall *method_call) {
    // method_call->instance->accept(this);
    for (auto &arg : method_call->args) {
      arg->accept(this);
    }
  }

  void visit_lambda(Lambda *lambda) {}

  void visit_namespace(NamespaceStmt *_namespace) {}

  void visit_scope_resolution(ScopeResolutionExpr *scope_resolution) {}
};