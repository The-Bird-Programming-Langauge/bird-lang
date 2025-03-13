#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "../bird_type.h"
#include "../core_call_table.h"
#include "../exceptions/bird_exception.h"
#include "../exceptions/user_error_tracker.h"
#include "../stack.h"
#include "../sym_table.h"
#include "../type_converter.h"
#include "ast_node/expr/method_call.h"
#include "hoist_visitor.h"

/*
 * Visitor that checks types of the AST
 */
class TypeChecker : public Visitor {
public:
  CoreCallTable core_call_table;

  Environment<std::shared_ptr<BirdType>> env;
  Environment<std::shared_ptr<BirdFunction>> call_table;

  Environment<std::shared_ptr<BirdType>> type_table;

  std::set<std::string> struct_names;
  bool found_return = false;

  Stack<std::shared_ptr<BirdType>> stack;
  std::optional<std::shared_ptr<BirdType>> return_type;
  UserErrorTracker &user_error_tracker;
  TypeConverter type_converter;

  std::unordered_map<
      std::string,
      std::unordered_map<std::string, std::shared_ptr<BirdFunction>>>
      v_table;

  TypeChecker(UserErrorTracker &user_error_tracker)
      : user_error_tracker(user_error_tracker),
        type_converter(this->type_table, this->struct_names) {
    this->env.push_env();
    this->call_table.push_env();
    this->type_table.push_env();
  }

  std::map<Token::Type, Token::Type> assign_to_binary_map = {
      {Token::Type::PLUS_EQUAL, Token::Type::PLUS},
      {Token::Type::MINUS_EQUAL, Token::Type::MINUS},
      {Token::Type::STAR_EQUAL, Token::Type::STAR},
      {Token::Type::SLASH_EQUAL, Token::Type::SLASH},
      {Token::Type::PERCENT_EQUAL, Token::Type::PERCENT},
  };

  // map of binary operations and their return types
  const std::map<Token::Type, std::map<std::pair<TypeTag, TypeTag>, TypeTag>>
      binary_operations = {
          {Token::Type::PLUS,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::INT},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::FLOAT},
               {{TypeTag::STRING, TypeTag::STRING}, TypeTag::STRING},
           }},
          {Token::Type::MINUS,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::INT},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::FLOAT},
           }},
          {Token::Type::STAR,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::INT},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::FLOAT},
           }},
          {Token::Type::SLASH,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::INT},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::FLOAT},
           }},
          {Token::Type::EQUAL_EQUAL,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::BOOL},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::BOOL},
               {{TypeTag::STRING, TypeTag::STRING}, TypeTag::BOOL},
               {{TypeTag::BOOL, TypeTag::BOOL}, TypeTag::BOOL},
           }},
          {Token::Type::BANG_EQUAL,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::BOOL},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::BOOL},
               {{TypeTag::STRING, TypeTag::STRING}, TypeTag::BOOL},
               {{TypeTag::BOOL, TypeTag::BOOL}, TypeTag::BOOL},
           }},
          {Token::Type::GREATER,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::BOOL},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::BOOL},
           }},
          {Token::Type::GREATER_EQUAL,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::BOOL},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::BOOL},
           }},
          {Token::Type::LESS,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::BOOL},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::BOOL},
           }},
          {Token::Type::LESS_EQUAL,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::BOOL},
               {{TypeTag::FLOAT, TypeTag::FLOAT}, TypeTag::BOOL},
           }},
          {Token::Type::AND,
           {
               {{TypeTag::BOOL, TypeTag::BOOL}, TypeTag::BOOL},
           }},
          {Token::Type::OR,
           {
               {{TypeTag::BOOL, TypeTag::BOOL}, TypeTag::BOOL},
           }},
          {Token::Type::XOR,
           {
               {{TypeTag::BOOL, TypeTag::BOOL}, TypeTag::BOOL},
           }},
          {Token::Type::PERCENT,
           {
               {{TypeTag::INT, TypeTag::INT}, TypeTag::INT},
           }},
      };

  void check_types(std::vector<std::unique_ptr<Stmt>> *stmts) {
    HoistVisitor hoist_visitor(this->struct_names);
    hoist_visitor.hoist(stmts);
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }

    while (!this->stack.empty()) {
      this->stack.pop();
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
    decl_stmt->value->accept(this);
    auto result = this->stack.pop();

    if (result->get_tag() == TypeTag::VOID) {
      this->user_error_tracker.type_error("cannot declare void type",
                                          decl_stmt->identifier);
      this->env.declare(decl_stmt->identifier.lexeme,
                        std::make_shared<ErrorType>());
      return;
    }

    if (decl_stmt->type.has_value()) {
      std::shared_ptr<BirdType> type =
          this->type_converter.convert(decl_stmt->type.value());
      if (*type != *result) {
        this->user_error_tracker.type_mismatch(
            "in declaration", decl_stmt->type.value()->get_token());

        this->env.declare(decl_stmt->identifier.lexeme,
                          std::make_unique<ErrorType>());
        return;
      }
    }

    this->env.declare(decl_stmt->identifier.lexeme, result);
  }

  void visit_assign_expr(AssignExpr *assign_expr) {
    if (!this->env.contains(assign_expr->identifier.lexeme)) {
      this->user_error_tracker.type_error("identifier not declared",
                                          assign_expr->identifier);
      this->env.set(assign_expr->identifier.lexeme,
                    std::make_shared<ErrorType>());
      return;
    }

    assign_expr->value->accept(this);
    auto result = this->stack.pop();

    auto previous = this->env.get(assign_expr->identifier.lexeme);

    if (assign_expr->assign_operator.token_type == Token::Type::EQUAL) {
      if (*previous != *result) {
        this->user_error_tracker.type_mismatch("in assignment",
                                               assign_expr->assign_operator);
        this->env.set(assign_expr->identifier.lexeme,
                      std::make_shared<ErrorType>());

        this->stack.push(std::make_shared<ErrorType>());
        return;
      }

      this->env.set(assign_expr->identifier.lexeme, result);

      this->stack.push(result);
      return;
    }

    auto binary_operator =
        this->assign_to_binary_map.at(assign_expr->assign_operator.token_type);
    auto type_map = this->binary_operations.at(binary_operator);

    if (type_map.find({previous->get_tag(), result->get_tag()}) ==
        type_map.end()) {
      this->user_error_tracker.type_mismatch("in assignment",
                                             assign_expr->assign_operator);
      this->env.set(assign_expr->identifier.lexeme,
                    std::make_shared<ErrorType>());

      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    auto new_type = type_map.at({previous->get_tag(), result->get_tag()});

    auto new_type_bird_type = bird_type_type_to_bird_type(new_type);
    this->env.set(assign_expr->identifier.lexeme, new_type_bird_type);

    this->stack.push(new_type_bird_type);
  }

  void visit_expr_stmt(ExprStmt *expr_stmt) { expr_stmt->expr->accept(this); }

  void visit_print_stmt(PrintStmt *print_stmt) {
    for (auto &arg : print_stmt->args) {
      arg->accept(this);
      auto result = this->stack.pop();

      if (result->get_tag() == TypeTag::VOID) {
        this->user_error_tracker.type_error("cannot print void type",
                                            print_stmt->print_token);
      }

      if (result->get_tag() == TypeTag::STRUCT) {
        this->user_error_tracker.type_error("cannot print struct type",
                                            print_stmt->print_token);
      }

      if (result->get_tag() == TypeTag::PLACEHOLDER) {
        this->user_error_tracker.type_error("cannot print struct type",
                                            print_stmt->print_token);
      }

      if (result->get_tag() == TypeTag::FUNCTION) {
        this->user_error_tracker.type_error("cannot print function type",
                                            print_stmt->print_token);
      }

      if (result->get_tag() == TypeTag::ERROR) {
        return;
      }
    }
  }

  void visit_const_stmt(ConstStmt *const_stmt) {
    const_stmt->value->accept(this);
    auto result = this->stack.pop();

    if (result->get_tag() == TypeTag::VOID) {
      this->user_error_tracker.type_error("cannot declare void type",
                                          const_stmt->identifier);
      this->env.declare(const_stmt->identifier.lexeme,
                        std::make_shared<ErrorType>());
      return;
    }

    if (const_stmt->type.has_value()) {
      std::shared_ptr<BirdType> type =
          this->type_converter.convert(const_stmt->type.value());

      if (*type != *result) {
        this->user_error_tracker.type_mismatch(
            "in declaration", const_stmt->type.value()->get_token());
        this->env.declare(const_stmt->identifier.lexeme,
                          std::make_shared<ErrorType>());
        return;
      }
    }

    this->env.declare(const_stmt->identifier.lexeme, result);
  }

  void visit_while_stmt(WhileStmt *while_stmt) {
    while_stmt->condition->accept(this);
    auto condition_result = this->stack.pop();

    if (condition_result->get_tag() != TypeTag::BOOL) {
      this->user_error_tracker.type_error(
          "expected bool in while statement condition",
          while_stmt->while_token);
    }

    while_stmt->stmt->accept(this);
  }

  void visit_for_stmt(ForStmt *for_stmt) {
    this->env.push_env();

    if (for_stmt->initializer.has_value()) {
      for_stmt->initializer.value()->accept(this);
    }

    if (for_stmt->condition.has_value()) {
      for_stmt->condition.value()->accept(this);
      auto condition_result = this->stack.pop();

      if (condition_result->get_tag() != TypeTag::BOOL) {
        this->user_error_tracker.type_error(
            "expected bool in for statement condition", for_stmt->for_token);
      }
    }

    if (for_stmt->increment.has_value()) {
      for_stmt->increment.value()->accept(this);
    }

    this->env.pop_env();
  }

  void visit_binary(Binary *binary) {
    binary->left->accept(this);
    binary->right->accept(this);

    auto right = this->stack.pop();
    auto left = this->stack.pop();

    auto operator_options = this->binary_operations.at(binary->op.token_type);
    if (operator_options.find({left->get_tag(), right->get_tag()}) ==
        operator_options.end()) {
      this->user_error_tracker.type_mismatch("in binary operation", binary->op);
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    this->stack.push(bird_type_type_to_bird_type(
        operator_options.at({left->get_tag(), right->get_tag()})));
  }

  void visit_unary(Unary *unary) {
    unary->expr->accept(this);
    auto result = this->stack.pop();
    switch (unary->op.token_type) {
    case Token::Type::MINUS: {
      if (result->get_tag() == TypeTag::FLOAT) {
        this->stack.push(std::make_shared<FloatType>());
      } else if (result->get_tag() == TypeTag::INT) {
        this->stack.push(std::make_shared<IntType>());
      } else {
        this->user_error_tracker.type_error(
            "expected int or float in unary operation, found: " +
                bird_type_to_string(result),
            unary->op);
        this->stack.push(std::make_shared<ErrorType>());
      }
      break;
    }
    case Token::Type::NOT: {
      if (result->get_tag() == TypeTag::BOOL) {
        this->stack.push(std::make_shared<BoolType>());
      } else {
        this->user_error_tracker.type_error(
            "expected bool int unary operation, found: " +
                bird_type_to_string(result),
            unary->op);
        this->stack.push(std::make_shared<ErrorType>());
      }
      break;
    }
    case Token::Type::QUESTION: {
      if (result->get_tag() == TypeTag::STRUCT) {
        this->stack.push(std::make_shared<BoolType>());
      } else if (result->get_tag() == TypeTag::PLACEHOLDER) {
        this->stack.push(std::make_shared<BoolType>());
      } else {
        this->user_error_tracker.type_error(
            "expected struct in unary operation, found: " +
                bird_type_to_string(result),
            unary->op);
        this->stack.push(std::make_shared<ErrorType>());
      }
      break;
    }
    default: {
      throw BirdException("Undefined unary operator, " + unary->op.lexeme);
    }
    }
  }

  void visit_primary(Primary *primary) {
    switch (primary->value.token_type) {
    case Token::Type::FLOAT_LITERAL: {
      this->stack.push(std::make_shared<FloatType>());
      break;
    }
    case Token::Type::INT_LITERAL: {
      this->stack.push(std::make_shared<IntType>());
      break;
    }
    case Token::Type::TRUE:
    case Token::Type::FALSE: {
      this->stack.push(std::make_shared<BoolType>());
      break;
    }
    case Token::Type::STR_LITERAL: {
      this->stack.push(std::make_shared<StringType>());
      break;
    }
    case Token::Type::IDENTIFIER: {
      this->stack.push(this->env.get(primary->value.lexeme));
      break;
    }
    case Token::Type::SELF: {
      this->stack.push(this->env.get("self"));
      break;
    }
    default: {
      throw BirdException("undefined primary value");
    }
    }
  }

  void visit_ternary(Ternary *ternary) {
    ternary->condition->accept(this);
    auto condition = this->stack.pop();

    ternary->true_expr->accept(this);
    auto true_expr = this->stack.pop();

    ternary->false_expr->accept(this);
    auto false_expr = this->stack.pop();

    if (*true_expr != *false_expr) {
      this->user_error_tracker.type_mismatch("in ternary operation",
                                             ternary->ternary_token);
      true_expr = std::make_shared<ErrorType>();
    }

    if (condition->get_tag() != TypeTag::BOOL) {
      this->user_error_tracker.type_error("expected bool in ternary condition",
                                          ternary->ternary_token);
      this->stack.push(std::make_shared<ErrorType>());
    } else {
      this->stack.push(true_expr);
    }
  }

  std::shared_ptr<BirdType> get_type_from_token(Token token) {
    auto type_name = token.lexeme;
    if (type_name == "int") {
      return std::make_shared<IntType>();
    } else if (type_name == "float") {
      return std::make_shared<FloatType>();
    } else if (type_name == "bool") {
      return std::make_shared<BoolType>();
    } else if (type_name == "str") {
      return std::make_shared<StringType>();
    } else if (type_name == "void") {
      return std::make_shared<VoidType>();
    } else {
      // type_name is not primitive
      if (this->type_table.contains(type_name)) {
        return this->type_table.get(type_name);
      }

      if (this->struct_names.find(type_name) != this->struct_names.end()) {
        return std::make_shared<PlaceholderType>(type_name);
      }

      this->user_error_tracker.type_error("unknown type", token);
      return std::make_shared<ErrorType>();
    }
  }

  std::shared_ptr<BirdFunction> create_func(Func *func) {
    std::vector<std::shared_ptr<BirdType>> params;

    std::transform(func->param_list.begin(), func->param_list.end(),
                   std::back_inserter(params), [&](auto param) {
                     return this->type_converter.convert(param.second);
                   });

    std::shared_ptr<BirdType> ret =
        func->return_type.has_value()
            ? this->type_converter.convert(func->return_type.value())
            : std::shared_ptr<BirdType>(new VoidType());

    return std::make_shared<BirdFunction>(params, ret);
  }

  void visit_func_helper(Func *func,
                         const std::shared_ptr<BirdFunction> bird_function) {
    this->found_return = false;
    auto previous_return_type = this->return_type;
    this->return_type = bird_function->ret;

    this->call_table.declare(func->identifier.lexeme, bird_function);
    this->env.push_env();

    for (auto &param : func->param_list) {
      this->env.declare(param.first.lexeme,
                        this->type_converter.convert(param.second));
    }

    for (auto &stmt : dynamic_cast<Block *>(func->block.get())
                          ->stmts) // TODO: figure out how not to dynamic cast
    {
      stmt->accept(this);
    }

    if (!this->found_return && func->return_type.has_value() &&
        func->return_type.value()->get_token().lexeme != "void") {
      this->user_error_tracker.type_error(
          "Missing return in a non-void function '" + func->identifier.lexeme +
              ".'",
          func->identifier);
    }

    this->return_type = previous_return_type;
    this->env.pop_env();
  }

  void visit_func(Func *func) {
    const auto bird_function = create_func(func);
    this->visit_func_helper(func, bird_function);
  }

  void visit_if_stmt(IfStmt *if_stmt) {
    if_stmt->condition->accept(this);
    auto condition = this->stack.pop();

    if (condition->get_tag() != TypeTag::BOOL) {
      this->user_error_tracker.type_error(
          "expected bool in if statement condition", if_stmt->if_token);
    }

    if_stmt->then_branch->accept(this);

    if (if_stmt->else_branch.has_value()) {
      if_stmt->else_branch.value()->accept(this);
    }
  }

  bool correct_arity(std::vector<std::shared_ptr<BirdType>> params,
                     std::vector<std::shared_ptr<Expr>> args) {
    return params.size() == args.size();
  }

  void compare_args_and_params(Token call_identifier,
                               std::vector<std::shared_ptr<Expr>> args,
                               std::vector<std::shared_ptr<BirdType>> params) {
    if (!correct_arity(params, args)) {
      this->user_error_tracker.type_error("Invalid number of arguments to " +
                                              call_identifier.lexeme,
                                          call_identifier);

      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    for (int i = 0; i < params.size(); i++) {
      args[i]->accept(this);
      auto arg = this->stack.pop();
      auto param = params[i];

      if (arg->get_tag() == TypeTag::PLACEHOLDER &&
          param->get_tag() == TypeTag::STRUCT) {
        auto placeholder_type = safe_dynamic_pointer_cast<PlaceholderType>(arg);
        auto struct_type = safe_dynamic_pointer_cast<StructType>(param);

        if (struct_type->name == placeholder_type->name) {
          continue;
        }
      }

      if (*arg != *param) {
        this->user_error_tracker.type_mismatch("in function call",
                                               call_identifier);
      }
    }
  }

  void visit_call(Call *call) {
    auto function = this->call_table.get(call->identifier.lexeme);
    compare_args_and_params(call->identifier, call->args, function->params);
    this->stack.push(function->ret);
  }

  void visit_return_stmt(ReturnStmt *return_stmt) {
    this->found_return = true;
    if (return_stmt->expr.has_value()) {
      return_stmt->expr.value()->accept(this);
      auto result = this->stack.pop();

      if (this->return_type.has_value()) {
        if (*result != *this->return_type.value()) {
          this->user_error_tracker.type_mismatch("in return statement",
                                                 return_stmt->return_token);
        }
      } else {
        this->user_error_tracker.type_error(
            "unexpected return value in void function",
            return_stmt->return_token);
      }
    } else {
      if (!this->return_type.has_value() ||
          this->return_type.value()->get_tag() != TypeTag::VOID) {
        this->user_error_tracker.type_error(
            "expected return value in non-void function",
            return_stmt->return_token);
      }
    }
  }

  void visit_break_stmt(BreakStmt *break_stmt) {
    // do nothing
  }

  void visit_continue_stmt(ContinueStmt *continue_stmt) {
    // do nothing
  }

  void visit_type_stmt(TypeStmt *type_stmt) {
    if (this->type_table.contains(type_stmt->identifier.lexeme)) {
      this->user_error_tracker.type_error("type already declared",
                                          type_stmt->identifier);
      return;
    }

    this->type_table.declare(
        type_stmt->identifier.lexeme,
        this->type_converter.convert(type_stmt->type_token));
  }

  void visit_subscript(Subscript *subscript) {
    subscript->subscriptable->accept(this);
    auto subscriptable = this->stack.pop();

    subscript->index->accept(this);
    auto index = this->stack.pop();

    if (subscriptable->get_tag() != TypeTag::STRING &&
        subscriptable->get_tag() != TypeTag::ARRAY) {
      this->user_error_tracker.type_error("expected string in subscriptable",
                                          subscript->subscript_token);

      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    if (index->get_tag() != TypeTag::INT) {
      this->user_error_tracker.type_error("expected int in subscript index",
                                          subscript->subscript_token);
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    if (subscriptable->get_tag() == TypeTag::STRING) {
      this->stack.push(std::make_shared<StringType>());
      return;
    }

    auto array_type = safe_dynamic_pointer_cast<ArrayType>(subscriptable);
    this->stack.push(array_type->element_type);
  }

  void visit_struct_decl(StructDecl *struct_decl) {
    std::vector<std::pair<std::string, std::shared_ptr<BirdType>>>
        struct_fields;
    std::transform(
        struct_decl->fields.begin(), struct_decl->fields.end(),
        std::back_inserter(struct_fields),
        [&](std::pair<Token, std::shared_ptr<ParseType::Type>> &field) {
          return std::make_pair(field.first.lexeme,
                                this->type_converter.convert(field.second));
        });

    // TODO: check invalid field types
    auto struct_type = std::make_shared<StructType>(
        struct_decl->identifier.lexeme, struct_fields);
    this->type_table.declare(struct_decl->identifier.lexeme, struct_type);

    for (auto &method : struct_decl->fns) {
      method->accept(this);
    }

    for (auto &method : struct_decl->fns) {
      const auto &bird_function =
          this->v_table.at(struct_decl->identifier.lexeme)
              .at(method->identifier.lexeme);
      this->visit_func_helper(method.get(), bird_function);
    }
  }

  void visit_direct_member_access(DirectMemberAccess *direct_member_access) {
    direct_member_access->accessable->accept(this);
    auto accessable = this->stack.pop();

    if (accessable->get_tag() == TypeTag::ERROR) {
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }
    if (accessable->get_tag() == TypeTag::PLACEHOLDER) {
      auto placeholder = safe_dynamic_pointer_cast<PlaceholderType>(accessable);
      if (this->struct_names.find(placeholder->name) ==
          this->struct_names.end()) {
        this->user_error_tracker.type_error("struct not declared", Token());
        this->stack.push(std::make_shared<ErrorType>());
        return;
      }

      accessable = this->type_table.get(placeholder->name);
    }

    if (accessable->get_tag() != TypeTag::STRUCT) {
      this->user_error_tracker.type_error(
          "expected struct in direct member access, found: " +
              bird_type_to_string(accessable),
          direct_member_access->identifier);
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    auto struct_type = safe_dynamic_pointer_cast<StructType>(accessable);

    for (auto &f : struct_type->fields) {
      if (f.first == direct_member_access->identifier.lexeme) {
        this->stack.push(f.second);
        return;
      }
    }

    this->user_error_tracker.type_error("field does not exist on struct",
                                        direct_member_access->identifier);
    this->stack.push(std::make_shared<ErrorType>());
    return;
  }

  void
  visit_struct_initialization(StructInitialization *struct_initialization) {
    if (!this->type_table.contains(struct_initialization->identifier.lexeme)) {
      this->user_error_tracker.type_error("struct not declared",
                                          struct_initialization->identifier);
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    auto type = this->type_table.get(struct_initialization->identifier.lexeme);

    auto struct_type = safe_dynamic_pointer_cast<StructType>(type);

    for (auto &field_assignment : struct_initialization->field_assignments) {
      auto found = false;
      for (auto &field : struct_type->fields) {
        if (field.first == field_assignment.first) {
          found = true;
          break;
        }
      }

      if (!found) {
        this->user_error_tracker.type_error(
            "field \"" + field_assignment.first +
                "\" does not exist in struct " +
                struct_initialization->identifier.lexeme,
            struct_initialization->identifier);
        this->stack.push(std::make_shared<ErrorType>());
        return;
      }
    }

    for (auto &field : struct_type->fields) {
      for (auto &field_assignment : struct_initialization->field_assignments) {
        field_assignment.second->accept(this);
        auto field_type = this->stack.pop();
        if (field.first == field_assignment.first) {
          if (field_type->get_tag() == TypeTag::ERROR) {
            this->stack.push(std::make_shared<ErrorType>());
            return;
          }

          if (field.second->get_tag() == TypeTag::PLACEHOLDER) {
            auto placeholder =
                safe_dynamic_pointer_cast<PlaceholderType>(field.second);
            if (this->struct_names.find(placeholder->name) ==
                this->struct_names.end()) {
              this->user_error_tracker.type_error("struct not declared",
                                                  Token());
              this->stack.push(std::make_shared<ErrorType>());
              return;
            }

            field.second = this->type_table.get(placeholder->name);
          }

          if (field_type->get_tag() == TypeTag::PLACEHOLDER) {
            auto placeholder =
                safe_dynamic_pointer_cast<PlaceholderType>(field_type);
            if (this->struct_names.find(placeholder->name) ==
                this->struct_names.end()) {
              this->user_error_tracker.type_error("struct not declared",
                                                  Token());
              this->stack.push(std::make_shared<ErrorType>());
              return;
            }

            field_type = this->type_table.get(placeholder->name);
          }

          if (*field.second != *field_type) {
            this->user_error_tracker.type_mismatch(
                "in struct initialization", struct_initialization->identifier);
            this->stack.push(std::make_shared<ErrorType>());
            return;
          }

          break;
        }
      }
    }

    this->stack.push(type);
  }

  void visit_member_assign(MemberAssign *member_assign) {
    member_assign->accessable->accept(this);
    auto accessable = this->stack.pop();

    if (accessable->get_tag() == TypeTag::ERROR) {
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    if (accessable->get_tag() != TypeTag::STRUCT) {
      this->user_error_tracker.type_error("expected struct in member assign",
                                          member_assign->identifier);
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    auto struct_type = safe_dynamic_pointer_cast<StructType>(accessable);

    for (auto &f : struct_type->fields) {
      if (f.first == member_assign->identifier.lexeme) {
        member_assign->value->accept(this);
        auto value = this->stack.pop();

        if (*f.second != *value) {
          this->user_error_tracker.type_mismatch("in member assign",
                                                 member_assign->identifier);
          this->stack.push(std::make_shared<ErrorType>());
          return;
        }

        this->stack.push(f.second);
        return;
      }
    }
  }

  void visit_as_cast(AsCast *as_cast) {
    as_cast->expr->accept(this);
    auto expr = this->stack.pop();

    if (expr->get_tag() == TypeTag::ERROR) {
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    auto to_type = this->type_converter.convert(as_cast->type);

    if (*to_type == *expr) {
      this->stack.push(to_type);
      return;
    }

    if (to_type->get_tag() == TypeTag::FLOAT &&
        expr->get_tag() == TypeTag::INT) {
      this->stack.push(to_type);
      return;
    }

    if (to_type->get_tag() == TypeTag::INT &&
        expr->get_tag() == TypeTag::FLOAT) {
      this->stack.push(to_type);
      return;
    }

    if (to_type->get_tag() == TypeTag::ARRAY &&
        expr->get_tag() == TypeTag::ARRAY) {
      auto to_type_array = safe_dynamic_pointer_cast<ArrayType>(to_type);
      auto expr_type_array = safe_dynamic_pointer_cast<ArrayType>(expr);

      if (expr_type_array->element_type->get_tag() == TypeTag::VOID) {
        this->stack.push(to_type);
        return;
      }
      if (*expr_type_array->element_type == *to_type_array->element_type) {
        this->stack.push(to_type);
        return;
      }
    }

    this->user_error_tracker.type_mismatch("in 'as' type cast",
                                           as_cast->type->get_token());
    this->stack.push(std::make_shared<ErrorType>());
  }

  void visit_array_init(ArrayInit *array_init) {
    auto elements = array_init->elements;
    if (!elements.size()) {
      this->stack.push(std::make_shared<ArrayType>(
          std::make_shared<VoidType>())); // resolved later?
      return;
    }

    elements[0]->accept(this);
    auto first_el_type = this->stack.pop();

    for (int i = 1; i < elements.size(); i++) {
      elements[i]->accept(this);
      auto type = this->stack.pop();

      if (*first_el_type != *type) {
        Token error_token;
        if (auto *primary_expr = dynamic_cast<Primary *>(elements[i].get())) {
          error_token = primary_expr->value;
        }

        this->user_error_tracker.type_mismatch("in array initialization",
                                               error_token);

        this->stack.push(std::make_shared<ErrorType>());
        return;
      }
    }

    this->stack.push(std::make_shared<ArrayType>(first_el_type));
  }

  void visit_index_assign(IndexAssign *index_assign) {
    index_assign->lhs->accept(this);
    auto lhs_type = this->stack.pop();

    index_assign->rhs->accept(this);
    auto rhs_type = this->stack.pop();

    if (lhs_type->get_tag() != rhs_type->get_tag()) {
      this->user_error_tracker.type_mismatch(
          "in assignment", index_assign->lhs->subscript_token);
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    this->stack.push(lhs_type);
  }

  void visit_match_expr(MatchExpr *match_expr) {
    match_expr->expr->accept(this);
    auto expr_type = this->stack.pop();

    match_expr->else_arm->accept(this);
    auto else_arm_type = this->stack.pop();

    if (else_arm_type->get_tag() == TypeTag::ERROR) {
      this->stack.push(std::make_shared<ErrorType>());
      return;
    }

    for (auto &arm : match_expr->arms) {
      arm.first->accept(this);
      auto arm_type = this->stack.pop();

      if (*arm_type != *expr_type) {
        this->user_error_tracker.type_mismatch("in match expression",
                                               match_expr->match_token);
        this->stack.push(std::make_shared<ErrorType>());
        return;
      }

      arm.second->accept(this);
      auto result_type = this->stack.pop();

      if (result_type->get_tag() == TypeTag::ERROR) {
        this->stack.push(std::make_shared<ErrorType>());
        return;
      }

      if (*else_arm_type != *result_type) {
        this->user_error_tracker.type_mismatch("in match expression",
                                               match_expr->match_token);
        this->stack.push(std::make_shared<ErrorType>());
        return;
      }
    }

    this->stack.push(else_arm_type);
  }

  void visit_method(Method *method) {
    this->v_table[method->class_identifier.lexeme][method->identifier.lexeme] =
        create_func(method);
  }

  void visit_method_call(MethodCall *method_call) {
    method_call->instance->accept(this);
    const auto struct_temp = this->stack.pop();
    if (struct_temp->get_tag() != TypeTag::STRUCT) {
      this->stack.push(std::make_shared<ErrorType>());
      this->user_error_tracker.type_error(
          "method '" + method_call->identifier.lexeme +
              "' does not exist on type " + struct_temp->to_string(),
          method_call->identifier);
      return;
    }

    const auto struct_type = std::dynamic_pointer_cast<StructType>(struct_temp);

    if (this->v_table.count(struct_type->name) == 0 ||
        this->v_table.at(struct_type->name)
                .count(method_call->identifier.lexeme) == 0) {
      this->user_error_tracker.type_error(
          "method '" + method_call->identifier.lexeme +
              "' does not exist on type " + struct_temp->to_string(),
          method_call->identifier);
      return;
    }

    const auto method =
        this->v_table.at(struct_type->name).at(method_call->identifier.lexeme);

    std::vector<std::shared_ptr<BirdType>> new_params;

    for (int i = 1; i < method->params.size(); i += 1) {
      new_params.push_back(method->params[i]);
    }

    this->compare_args_and_params(method_call->identifier, method_call->args,
                                  new_params);

    this->stack.push(method->ret);
  }
};
