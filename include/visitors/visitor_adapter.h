#pragma once
#include "../ast_node/index.h"
#include "visitor.h"

class VisitorAdapter : public Visitor {
public:
  void visit_binary(Binary *binary) override {
    binary->left->accept(this);
    binary->right->accept(this);
  }
  void visit_unary(Unary *unary) override { unary->expr->accept(this); }
  void visit_primary(Primary *primary) override {}
  void visit_ternary(Ternary *ternary) override {
    ternary->condition->accept(this);
    ternary->true_expr->accept(this);
    ternary->false_expr->accept(this);
  }
  void visit_decl_stmt(DeclStmt *decl_stmt) override {
    decl_stmt->value->accept(this);
  }
  void visit_assign_expr(AssignExpr *assign_expr) override {
    assign_expr->value->accept(this);
  }
  void visit_expr_stmt(ExprStmt *expr_stmt) override {
    expr_stmt->expr->accept(this);
  }
  void visit_print_stmt(PrintStmt *print_stmt) override {
    for (auto arg : print_stmt->args) {
      arg->accept(this);
    }
  }
  void visit_const_stmt(ConstStmt *const_stmt) override {
    const_stmt->value->accept(this);
  }
  void visit_func(Func *func) override { func->block->accept(this); }
  void visit_if_stmt(IfStmt *if_stmt) override {
    if_stmt->condition->accept(this);
    if_stmt->then_branch->accept(this);
    if (if_stmt->else_branch.has_value()) {
      if_stmt->else_branch.value()->accept(this);
    }
  }
  void visit_while_stmt(WhileStmt *while_stmt) override {
    while_stmt->condition->accept(this);
    while_stmt->stmt->accept(this);
  }
  void visit_for_stmt(ForStmt *for_stmt) override {
    if (for_stmt->initializer.has_value()) {
      for_stmt->initializer.value()->accept(this);
    }
    if (for_stmt->condition.has_value()) {
      for_stmt->condition.value()->accept(this);
    }
    if (for_stmt->increment.has_value()) {
      for_stmt->increment.value()->accept(this);
    }

    for_stmt->body->accept(this);
  }
  void visit_block(Block *block) override {
    for (auto &stmt : block->stmts) {
      stmt->accept(this);
    }
  }
  void visit_call(Call *call) override {
    for (auto arg : call->args) {
      arg->accept(this);
    }
  }
  void visit_return_stmt(ReturnStmt *return_stmt) override {
    if (return_stmt->expr.has_value()) {
      return_stmt->expr.value()->accept(this);
    }
  }

  void visit_break_stmt(BreakStmt *) override {}
  void visit_continue_stmt(ContinueStmt *) override {}
  void visit_type_stmt(TypeStmt *) override {}

  void visit_subscript(Subscript *subscript) override {
    subscript->subscriptable->accept(this);
    subscript->index->accept(this);
  }
  void visit_struct_decl(StructDecl *struct_decl) override {
    for (auto fn : struct_decl->fns) {
      this->visit_method(fn.get());
    }
  }
  void visit_direct_member_access(
      DirectMemberAccess *direct_member_access) override {
    direct_member_access->accessable->accept(this);
  }
  void visit_struct_initialization(StructInitialization *struct_init) override {
    for (auto &[name, expr] : struct_init->field_assignments) {
      expr->accept(this);
    }
  }
  void visit_member_assign(MemberAssign *member_assign) override {
    member_assign->accessable->accept(this);
    member_assign->value->accept(this);
  }
  void visit_as_cast(AsCast *as_cast) override { as_cast->expr->accept(this); }
  void visit_array_init(ArrayInit *array_init) override {
    for (auto el : array_init->elements) {
      el->accept(this);
    }
  }
  void visit_index_assign(IndexAssign *index_assign) override {
    index_assign->lhs->accept(this);
    index_assign->rhs->accept(this);
  }
  void visit_match_expr(MatchExpr *match_expr) override {
    match_expr->expr->accept(this);
    for (auto &[match, result] : match_expr->arms) {
      match->accept(this);
      result->accept(this);
    }

    match_expr->else_arm->accept(this);
  }

  void visit_method(Method *method) override { this->visit_func(method); }

  void visit_method_call(MethodCall *method_call) override {
    method_call->instance->accept(this);
    for (auto arg : method_call->args) {
      arg->accept(this);
    }
  }

  void visit_import_stmt(ImportStmt *import_stmt) override {}

  void visit_namespace(NamespaceStmt *_namespace) override {
    for (auto &member : _namespace->members) {
      member->accept(this);
    }
  }

  void visit_scope_resolution(ScopeResolutionExpr *scope_resolution) override {
    scope_resolution->identifier->accept(this);
  }

  void visit_for_in_stmt(ForInStmt *for_in) override {
    for_in->iterable->accept(this);
  }
};