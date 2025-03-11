#pragma once
#include "visitor.h"

class VisitorAdapter : public Visitor {
public:
  void visit_binary(Binary *) override {}
  void visit_unary(Unary *) override {}
  void visit_primary(Primary *) override {}
  void visit_ternary(Ternary *) override {}
  void visit_decl_stmt(DeclStmt *) override {}
  void visit_assign_expr(AssignExpr *) override {}
  void visit_expr_stmt(ExprStmt *) override {}
  void visit_print_stmt(PrintStmt *) override {}
  void visit_const_stmt(ConstStmt *) override {}
  void visit_func(Func *) override {}
  void visit_if_stmt(IfStmt *) override {}
  void visit_while_stmt(WhileStmt *) override {}
  void visit_for_stmt(ForStmt *) override {}
  void visit_block(Block *) override {}
  void visit_call(Call *) override {}
  void visit_return_stmt(ReturnStmt *) override {}
  void visit_break_stmt(BreakStmt *) override {}
  void visit_continue_stmt(ContinueStmt *) override {}
  void visit_type_stmt(TypeStmt *) override {}
  void visit_subscript(Subscript *) override {}
  void visit_struct_decl(StructDecl *) override {}
  void visit_direct_member_access(DirectMemberAccess *) override {}
  void visit_struct_initialization(StructInitialization *) override {}
  void visit_member_assign(MemberAssign *) override {}
  void visit_as_cast(AsCast *) override {}
  void visit_array_init(ArrayInit *) override {}
  void visit_index_assign(IndexAssign *) override {}
  void visit_match_expr(MatchExpr *) override {}
  void visit_import_stmt(ImportStmt *) override {}
};