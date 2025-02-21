#pragma once

#include <memory>
#include <set>
#include <variant>
#include <vector>

#include "ast_node/index.h"

#include "callable.h"
#include "exceptions/bird_exception.h"
#include "exceptions/break_exception.h"
#include "exceptions/continue_exception.h"
#include "exceptions/return_exception.h"
#include "exceptions/user_error_tracker.h"
#include "sym_table.h"
#include "value.h"

/*
 * Visitor that analyzes semantics of the AST
 */
class SemanticAnalyzer : public Visitor {

public:
  Environment<SemanticValue> env;
  Environment<SemanticCallable> call_table;
  Environment<SemanticType> type_table;
  UserErrorTracker &user_error_tracker;
  int loop_depth;
  int function_depth;
  bool found_return;

  SemanticAnalyzer(UserErrorTracker &user_error_tracker)
      : user_error_tracker(user_error_tracker) {
    this->env.push_env();
    this->call_table.push_env();
    this->type_table.push_env();
    this->loop_depth = 0;
    this->function_depth = 0;
  }

  void analyze_semantics(std::vector<std::unique_ptr<Stmt>> *stmts) {
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }
  }

  void visit_block(Block *block) {
    this->env.push_env();

    for (auto &stmt : block->stmts) {
      stmt->accept(this);
    }

    this->env.pop_env();
  }

  void visit_decl_stmt(DeclStmt *decl_stmt) {
    if (this->identifer_in_any_environment(decl_stmt->identifier.lexeme)) {
      this->user_error_tracker.semantic_error("Identifier '" +
                                                  decl_stmt->identifier.lexeme +
                                                  "' is already declared.",
                                              decl_stmt->identifier);
      return;
    }

    decl_stmt->value->accept(this);

    SemanticValue mutable_value;
    mutable_value.is_mutable = true;
    this->env.declare(decl_stmt->identifier.lexeme, mutable_value);
  }

  void visit_assign_expr(AssignExpr *assign_expr) {
    if (!this->env.contains(assign_expr->identifier.lexeme) &&
        !this->call_table.contains(assign_expr->identifier.lexeme)) {
      this->user_error_tracker.semantic_error(
          "Variable '" + assign_expr->identifier.lexeme + "' does not exist.",
          assign_expr->identifier);
      return;
    }

    auto previous_value = this->env.get(assign_expr->identifier.lexeme);

    if (!previous_value.is_mutable) {
      this->user_error_tracker.semantic_error(
          "Identifier '" + assign_expr->identifier.lexeme + "' is not mutable.",
          assign_expr->identifier);
      return;
    }

    assign_expr->value->accept(this);
  }

  void visit_expr_stmt(ExprStmt *expr_stmt) { expr_stmt->expr->accept(this); }

  void visit_print_stmt(PrintStmt *print_stmt) {
    for (auto &arg : print_stmt->args) {
      arg->accept(this);
    }
  }

  void visit_const_stmt(ConstStmt *const_stmt) {
    if (this->identifer_in_any_environment(const_stmt->identifier.lexeme)) {
      this->user_error_tracker.semantic_error(
          "Identifier '" + const_stmt->identifier.lexeme +
              "' is already declared.",
          const_stmt->identifier);
      return;
    }

    const_stmt->value->accept(this);

    this->env.declare(const_stmt->identifier.lexeme, SemanticValue());
  }

  void visit_while_stmt(WhileStmt *while_stmt) {
    this->loop_depth += 1;

    while_stmt->condition->accept(this);
    while_stmt->stmt->accept(this);

    this->loop_depth -= 1;
  }

  void visit_for_stmt(ForStmt *for_stmt) {
    this->loop_depth += 1;
    this->env.push_env();

    if (for_stmt->initializer.has_value()) {
      for_stmt->initializer.value()->accept(this);
    }

    if (for_stmt->condition.has_value()) {
      for_stmt->condition.value()->accept(this);
    }

    for_stmt->body->accept(this);

    if (for_stmt->increment.has_value()) {
      for_stmt->increment.value()->accept(this);
    }

    this->env.pop_env();

    this->loop_depth -= 1;
  }

  void visit_binary(Binary *binary) {
    binary->left->accept(this);
    binary->right->accept(this);
  }

  void visit_unary(Unary *unary) { unary->expr->accept(this); }

  void visit_primary(Primary *primary) {
    if (primary->value.token_type == Token::Type::IDENTIFIER &&
        !this->env.contains(primary->value.lexeme)) {
      this->user_error_tracker.semantic_error(
          "Variable '" + primary->value.lexeme + "' does not exist.",
          primary->value);
      return;
    }
  }

  void visit_ternary(Ternary *ternary) {
    ternary->condition->accept(this);
    ternary->true_expr->accept(this);
    ternary->false_expr->accept(this);
  }

  void visit_func(Func *func) {
    this->function_depth += 1;
    this->found_return = false;

    if (this->identifer_in_any_environment(func->identifier.lexeme)) {
      this->user_error_tracker.semantic_error(
          "Identifier '" + func->identifier.lexeme + "' is already declared.",
          func->identifier);
      return;
    }

    this->call_table.declare(func->identifier.lexeme,
                             SemanticCallable(func->param_list.size()));

    this->env.push_env();

    for (auto &param : func->param_list) {
      this->env.declare(param.first.lexeme, SemanticValue());
    }

    auto block = std::dynamic_pointer_cast<Block>(func->block);
    for (auto &stmt : block->stmts) {
      stmt->accept(this);
    }

    if (!found_return && func->return_type.has_value() &&
        func->return_type.value()->get_token().lexeme != "void") {
      this->user_error_tracker.semantic_error(
          "Function '" + func->identifier.lexeme +
              "' does not have a return statement.",
          func->identifier);
    }

    this->env.pop_env();

    this->function_depth -= 1;
  }

  void visit_if_stmt(IfStmt *if_stmt) {
    if_stmt->condition->accept(this);
    if_stmt->then_branch->accept(this);

    if (if_stmt->else_branch.has_value()) {
      if_stmt->else_branch.value()->accept(this);
    }
  }

  void visit_call(Call *call) {
    if (!this->call_table.contains(call->identifier.lexeme)) {
      this->user_error_tracker.semantic_error("Function call identifier '" +
                                                  call->identifier.lexeme +
                                                  "' is not declared.",
                                              call->identifier);
      return;
    }

    auto function = this->call_table.get(call->identifier.lexeme);

    if (function.param_count != call->args.size()) {
      this->user_error_tracker.semantic_error(
          "Function call identifer '" + call->identifier.lexeme +
              "' does not use the correct number of arguments.",
          call->identifier);
      return;
    }
  }

  void visit_return_stmt(ReturnStmt *return_stmt) {
    this->found_return = true;
    if (this->function_depth == 0) {
      this->user_error_tracker.semantic_error(
          "Return statement is declared outside of a function.",
          return_stmt->return_token);
      return;
    }

    if (return_stmt->expr.has_value()) {
      return_stmt->expr.value()->accept(this);
    }
  }

  void visit_break_stmt(BreakStmt *break_stmt) {
    if (this->loop_depth == 0) {
      this->user_error_tracker.semantic_error(
          "Break statement is declared outside of a loop.",
          break_stmt->break_token);
      return;
    }
  }

  void visit_continue_stmt(ContinueStmt *continue_stmt) {
    if (this->loop_depth == 0) {
      this->user_error_tracker.semantic_error(
          "Continue statement is declared outside of a loop.",
          continue_stmt->continue_token);
      return;
    }
  }

  void visit_type_stmt(TypeStmt *type_stmt) {
    if (this->identifer_in_any_environment(type_stmt->identifier.lexeme)) {
      this->user_error_tracker.semantic_error("Identifier '" +
                                                  type_stmt->identifier.lexeme +
                                                  "' is already declared.",
                                              type_stmt->identifier);
      return;
    }

    this->type_table.declare(type_stmt->identifier.lexeme, SemanticType());
  }

  bool identifer_in_any_environment(std::string identifer) {
    return this->env.current_contains(identifer) ||
           this->call_table.current_contains(identifer) ||
           this->type_table.current_contains(identifer);
  }

  void visit_subscript(Subscript *subscript) {
    subscript->subscriptable->accept(this);
    subscript->index->accept(this);
  };

  void visit_struct_decl(StructDecl *struct_decl) {
    this->type_table.declare(struct_decl->identifier.lexeme, SemanticType());
  }

  void visit_direct_member_access(DirectMemberAccess *direct_member_access) {
    direct_member_access->accessable->accept(this);
  }

  void
  visit_struct_initialization(StructInitialization *struct_initialization) {
    for (auto &field_assignment : struct_initialization->field_assignments) {
      field_assignment.second->accept(this);
    }
  }

  void visit_member_assign(MemberAssign *member_assign) {
    member_assign->accessable->accept(this);
  }

  void visit_as_cast(AsCast *as_cast) { as_cast->expr->accept(this); }

  void visit_array_init(ArrayInit *array_init) {
    for (auto &el : array_init->elements) {
      el->accept(this);
    }
  }

  void visit_index_assign(IndexAssign *index_assign) {
    index_assign->lhs->accept(this);
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
};