#pragma once
#include <memory>
#include <string>

// forward declarations
class Binary;
class Unary;
class Primary;
class Ternary;
class DeclStmt;
class AssignExpr;
class ExprStmt;
class PrintStmt;
class ConstStmt;
class WhileStmt;
class ForStmt;
class Block;
class IfStmt;
class Func;
class Call;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class TypeStmt;
class Subscript;
class StructDecl;
class DirectMemberAccess;
class StructInitialization;
class MemberAssign;
class AsCast;
class ArrayInit;
class IndexAssign;

/*
 * the interface for all visitors,
 * used through the AST nodes' `accept` function
 */
class Visitor {
public:
  ~Visitor() = default;
  virtual void visit_binary(Binary *) = 0;
  virtual void visit_unary(Unary *) = 0;
  virtual void visit_primary(Primary *) = 0;
  virtual void visit_ternary(Ternary *) = 0;
  virtual void visit_decl_stmt(DeclStmt *) = 0;
  virtual void visit_assign_expr(AssignExpr *) = 0;
  virtual void visit_expr_stmt(ExprStmt *) = 0;
  virtual void visit_print_stmt(PrintStmt *) = 0;
  virtual void visit_const_stmt(ConstStmt *) = 0;
  virtual void visit_func(Func *) = 0;
  virtual void visit_if_stmt(IfStmt *) = 0;
  virtual void visit_while_stmt(WhileStmt *) = 0;
  virtual void visit_for_stmt(ForStmt *) = 0;
  virtual void visit_block(Block *) = 0;
  virtual void visit_call(Call *) = 0;
  virtual void visit_return_stmt(ReturnStmt *) = 0;
  virtual void visit_break_stmt(BreakStmt *) = 0;
  virtual void visit_continue_stmt(ContinueStmt *) = 0;
  virtual void visit_type_stmt(TypeStmt *) = 0;
  virtual void visit_subscript(Subscript *) = 0;
  virtual void visit_struct_decl(StructDecl *) = 0;
  virtual void visit_direct_member_access(DirectMemberAccess *) = 0;
  virtual void visit_struct_initialization(StructInitialization *) = 0;
  virtual void visit_member_assign(MemberAssign *) = 0;
  virtual void visit_as_cast(AsCast *) = 0;
  virtual void visit_array_init(ArrayInit *) = 0;
  virtual void visit_index_assign(IndexAssign *) = 0;
};
