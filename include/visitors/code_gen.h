#pragma once

#include "../ast_node/index.h"
#include "../bird_type.h"
#include "../stack.h"
#include "../sym_table.h"
#include "../type_converter.h"
#include "binaryen-c.h"
#include "visitor.h"
#include <map>
#include <memory>

Token::Type assign_expr_binary_equivalent(Token::Type token_type);
unsigned int bird_type_byte_size(std::shared_ptr<BirdType> type);
BinaryenType bird_type_to_binaryen_type(std::shared_ptr<BirdType> bird_type);
const char *get_mem_get_for_type(const TypeTag type);
const char *get_mem_set_for_type(const TypeTag type);
bool type_is_on_heap(const TypeTag type);

template <typename T> struct Tagged {
  T value;
  std::shared_ptr<BirdType> type = std::shared_ptr<BirdType>(new VoidType());

  Tagged() : value(T()) {}
  Tagged(T v) : value(v) {}
  Tagged(T v, std::shared_ptr<BirdType> t) : value(v), type(t) {}
};

using TaggedExpression = Tagged<BinaryenExpressionRef>;
using TaggedIndex = Tagged<BinaryenIndex>;
using TaggedType = Tagged<BinaryenType>;
using TaggedBinaryOpFn = Tagged<std::function<BinaryenOp(void)>>;

class CodeGen : public Visitor {
  // and, or, and string operations are handled separately
  // differently
  const std::map<Token::Type,
                 std::map<std::pair<TypeTag, TypeTag>, TaggedBinaryOpFn>>
      binary_operations = {
          {Token::Type::PLUS,
           {{{TypeTag::INT, TypeTag::INT},
             TaggedBinaryOpFn(BinaryenAddInt32, std::make_shared<IntType>())},
            {{TypeTag::FLOAT, TypeTag::FLOAT},
             TaggedBinaryOpFn(BinaryenAddFloat64,
                              std::make_shared<FloatType>())}}},
          {Token::Type::MINUS,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenSubInt32,
                                 std::make_shared<IntType>())},
               {{TypeTag::FLOAT, TypeTag::FLOAT},
                TaggedBinaryOpFn(BinaryenSubFloat64,
                                 std::make_shared<FloatType>())},
           }},
          {Token::Type::STAR,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenMulInt32,
                                 std::make_shared<IntType>())},
               {{TypeTag::FLOAT, TypeTag::FLOAT},
                TaggedBinaryOpFn(BinaryenMulFloat64,
                                 std::make_shared<FloatType>())},
           }},
          {Token::Type::SLASH,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenDivSInt32,
                                 std::make_shared<IntType>())},
               {{TypeTag::FLOAT, TypeTag::FLOAT},
                TaggedBinaryOpFn(BinaryenDivFloat64,
                                 std::make_shared<FloatType>())},
           }},
          {Token::Type::EQUAL_EQUAL,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenEqInt32,
                                 std::make_shared<BoolType>())},
               {{TypeTag::FLOAT, TypeTag::FLOAT},
                TaggedBinaryOpFn(BinaryenEqFloat64,
                                 std::make_shared<BoolType>())},
               {{TypeTag::BOOL, TypeTag::BOOL},
                TaggedBinaryOpFn(BinaryenEqInt32,
                                 std::make_shared<BoolType>())},
           }},
          {Token::Type::BANG_EQUAL,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenNeInt32,
                                 std::make_shared<BoolType>())},
               {{TypeTag::FLOAT, TypeTag::FLOAT},
                TaggedBinaryOpFn(BinaryenNeFloat64,
                                 std::make_shared<BoolType>())},
               {{TypeTag::BOOL, TypeTag::BOOL},
                TaggedBinaryOpFn(BinaryenNeInt32,
                                 std::make_shared<BoolType>())},
           }},
          {Token::Type::GREATER,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenGtSInt32,
                                 std::make_shared<BoolType>())},
               {{TypeTag::FLOAT, TypeTag::FLOAT},
                TaggedBinaryOpFn(BinaryenGtFloat64,
                                 std::make_shared<BoolType>())},
           }},
          {Token::Type::GREATER_EQUAL,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenGeSInt32,
                                 std::make_shared<BoolType>())},
               {{TypeTag::FLOAT, TypeTag::FLOAT},
                TaggedBinaryOpFn(BinaryenGeFloat64,
                                 std::make_shared<BoolType>())},
           }},
          {Token::Type::LESS,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenLtSInt32,
                                 std::make_shared<BoolType>())},
               {{TypeTag::FLOAT, TypeTag::FLOAT},
                TaggedBinaryOpFn(BinaryenLtFloat64,
                                 std::make_shared<BoolType>())},
           }},
          {Token::Type::LESS_EQUAL,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenLeSInt32,
                                 std::make_shared<BoolType>())},
               {{TypeTag::FLOAT, TypeTag::FLOAT},
                TaggedBinaryOpFn(BinaryenLeFloat64,
                                 std::make_shared<BoolType>())},
           }},
          {Token::Type::XOR,
           {
               {{TypeTag::BOOL, TypeTag::BOOL},
                TaggedBinaryOpFn(BinaryenXorInt32,
                                 std::make_shared<BoolType>())},
           }},
          {Token::Type::PERCENT,
           {
               {{TypeTag::INT, TypeTag::INT},
                TaggedBinaryOpFn(BinaryenRemSInt32,
                                 std::make_shared<IntType>())},
           }},
      };

  Environment<TaggedIndex> environment; // tracks the index of local variables
  Environment<std::shared_ptr<BirdType>> type_table;
  Stack<TaggedExpression> stack; // for returning values
  std::map<std::string, std::string> std_lib;
  std::set<std::string> struct_names;
  std::unordered_map<std::string, int> struct_name_to_num_pointers;

  std::unordered_map<std::string, uint32_t> str_offsets;

  // we need the function return types when calling functions
  std::unordered_map<std::string, TaggedType> function_return_types;
  // allows us to track the local variables of a function
  std::unordered_map<std::string, std::vector<BinaryenType>> function_locals;
  std::unordered_map<std::string, int> function_param_count;
  std::string current_function_name; // for indexing into maps

  TypeConverter type_converter;

  BinaryenModuleRef mod;

  void init_std_lib();
  void init_static_memory();

  void visit_block(Block *block);
  void visit_decl_stmt(DeclStmt *decl_stmt);
  void visit_assign_expr(AssignExpr *assign_expr);
  void visit_print_stmt(PrintStmt *print_stmt);
  void visit_expr_stmt(ExprStmt *expr_stmt);
  void visit_while_stmt(WhileStmt *while_stmt);
  void visit_for_stmt(ForStmt *for_stmt);
  void visit_binary(Binary *binary);
  void visit_binary_short_circuit(Token::Type op, std::unique_ptr<Expr> &left,
                                  std::unique_ptr<Expr> &right);
  void visit_binary_normal(Token::Type op, TaggedExpression left,
                           TaggedExpression right);
  void create_binary(Token::Type op, std::unique_ptr<Expr> &left,
                     std::unique_ptr<Expr> &right);
  void handle_binary_string_operations(Token::Type op, TaggedExpression left,
                                       TaggedExpression right);
  void visit_unary(Unary *unary);
  void visit_primary(Primary *primary);
  TaggedExpression generate_string_from_string(std::string string);
  TaggedExpression
  generate_string_from_exprs(std::vector<BinaryenExpressionRef> vals);

  TaggedExpression create_unary_not(BinaryenExpressionRef condition);
  void visit_ternary(Ternary *ternary);
  void visit_const_stmt(ConstStmt *const_stmt);
  void visit_func(Func *func);
  void add_func_with_name(Func *func, std::string func_name);
  void visit_if_stmt(IfStmt *if_stmt);
  void visit_call(Call *call);
  void visit_return_stmt(ReturnStmt *return_stmt);
  void visit_break_stmt(BreakStmt *break_stmt);
  void visit_continue_stmt(ContinueStmt *continue_stmt);
  void visit_type_stmt(TypeStmt *type_stmt);
  void visit_subscript(Subscript *subscript);
  void visit_namespace(NamespaceStmt *_namespace);
  void visit_scope_resolution(ScopeResolutionExpr *scope_resolution);
  void visit_for_in_stmt(ForInStmt *for_in);

  /*
  This function is called when a struct declaration is encountered.
  The struct's shape is stored in the type table.
  */
  void visit_struct_decl(StructDecl *struct_decl);
  void visit_direct_member_access(DirectMemberAccess *direct_member_access);

  // map from struct name to function name
  std::unordered_map<std::string, std::string> struct_constructors;
  void visit_struct_initialization(StructInitialization *struct_initialization);
  void visit_member_assign(MemberAssign *member_assign);
  void visit_as_cast(AsCast *as_cast);

  TaggedExpression binaryen_set(std::string identifier,
                                BinaryenExpressionRef value);
  TaggedExpression binaryen_get(std::string identifier);

  void visit_array_init(ArrayInit *array_init);
  void visit_index_assign(IndexAssign *index_assign);

  void visit_match_expr(MatchExpr *match_expr);
  TaggedExpression match_helper(TaggedExpression expr, MatchExpr *match_expr,
                                int index);

  void visit_import_stmt(ImportStmt *import_stmt);

  void visit_method(Method *method);
  void visit_method_call(MethodCall *method_call);
  TaggedExpression create_call_with(std::string function_name,
                                    std::vector<BinaryenExpressionRef> args);
  void generate_array_length_fn();
  void generate_iter_fn();
  TaggedExpression convert_string_to_array(BinaryenExpressionRef str_ptr);

  void create_struct_constructor(std::shared_ptr<StructType> type);
  void init_array_constructor();
  void init_ref_constructor();
  BinaryenExpressionRef get_array_data(BinaryenExpressionRef &subscriptable);
  BinaryenExpressionRef deref(BinaryenExpressionRef &ref);
  BinaryenExpressionRef
  get_subscript_result(Tagged<BinaryenExpressionRef> &subscriptable,
                       Tagged<BinaryenExpressionRef> &index,
                       std::shared_ptr<BirdType> type);

public:
  ~CodeGen();
  CodeGen();
  void generate(std::vector<std::unique_ptr<Stmt>> *stmts);

  // for testing
  Environment<TaggedIndex> &get_environment();
  Environment<std::shared_ptr<BirdType>> &get_type_table();
};
