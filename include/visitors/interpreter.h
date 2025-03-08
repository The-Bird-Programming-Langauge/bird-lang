#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <variant>
#include <vector>
#include <list>

#include "../ast_node/index.h"
#include "hoist_visitor.h"

#include "../bird_type.h"
#include "../callable.h"
#include "../exceptions/bird_exception.h"
#include "../exceptions/break_exception.h"
#include "../exceptions/continue_exception.h"
#include "../exceptions/return_exception.h"
#include "../stack.h"
#include "../sym_table.h"
#include "../type.h"
#include "../type_converter.h"
#include "../value.h"

template <typename T, typename U, typename V>
struct Namespace
{
  Environment<T> environment;
  Environment<U> call_table;
  Environment<V> type_table;

  std::shared_ptr<Namespace> parent = nullptr; // allows for backtracking to higher namespaces
  std::unordered_map<std::string, std::shared_ptr<Namespace>> nested_namespaces;

  Namespace(std::shared_ptr<Namespace> parent_ns) : parent(parent_ns)
  {
    environment.push_env();
    call_table.push_env();
    type_table.push_env();
  }
};

/*
 * Visitor that interprets and evaluates the AST
 */
class Interpreter : public Visitor
{

public:
  std::shared_ptr<Namespace<Value, Callable, std::shared_ptr<BirdType>>> global_namespace;
  std::shared_ptr<Namespace<Value, Callable, std::shared_ptr<BirdType>>> current_namespace;

  Stack<Value> stack;
  std::set<std::string> struct_names;
  TypeConverter type_converter;

  Interpreter()
      : global_namespace(std::make_shared<Namespace<Value, Callable, std::shared_ptr<BirdType>>>(nullptr)),
        current_namespace(global_namespace),
        type_converter(current_namespace->type_table, struct_names) {}

  void evaluate(std::vector<std::unique_ptr<Stmt>> *stmts)
  {
    HoistVisitor hoist_visitor(this->struct_names);
    hoist_visitor.hoist(stmts);

    for (auto &stmt : *stmts)
    {
      stmt->accept(this);
    }

    while (!this->stack.empty())
    {
      this->stack.pop();
    }
  }

  void visit_block(Block *block)
  {
    this->current_namespace->environment.push_env();
    for (auto &stmt : block->stmts)
    {
      stmt->accept(this);
    }

    this->current_namespace->environment.pop_env();
  }

  void visit_decl_stmt(DeclStmt *decl_stmt)
  {
    decl_stmt->value->accept(this);

    auto result = std::move(this->stack.pop());
    result.is_mutable = true;

    this->current_namespace->environment.declare(decl_stmt->identifier.lexeme, std::move(result));
  }

  void visit_assign_expr(AssignExpr *assign_expr)
  {
    auto previous_value = this->current_namespace->environment.get(assign_expr->identifier.lexeme);

    assign_expr->value->accept(this);
    auto value = std::move(this->stack.pop());

    switch (assign_expr->assign_operator.token_type)
    {
    case Token::Type::EQUAL:
    {
      previous_value = value;
      break;
    }
    case Token::Type::PLUS_EQUAL:
    {
      previous_value = previous_value + value;
      break;
    }
    case Token::Type::MINUS_EQUAL:
    {
      previous_value = previous_value - value;
      break;
    }
    case Token::Type::STAR_EQUAL:
    {
      previous_value = previous_value * value;
      break;
    }
    case Token::Type::SLASH_EQUAL:
    {
      previous_value = previous_value / value;
      break;
    }
    case Token::Type::PERCENT_EQUAL:
    {
      previous_value = previous_value % value;
      break;
    }
    default:
      throw std::runtime_error("Unidentified assignment operator " +
                               assign_expr->assign_operator.lexeme);
    }

    this->current_namespace->environment.set(assign_expr->identifier.lexeme, previous_value);
  }

  void visit_expr_stmt(ExprStmt *expr_stmt) { expr_stmt->expr->accept(this); }

  void visit_print_stmt(PrintStmt *print_stmt)
  {
    for (auto &arg : print_stmt->args)
    {
      arg->accept(this);
      auto result = std::move(this->stack.pop());

      if (is_type<bool>(result))
      {
        std::cout << (as_type<bool>(result) ? "true" : "false");
      }
      else
      {
        std::cout << result;
      }
    }
    std::cout << std::endl;
  }

  void visit_const_stmt(ConstStmt *const_stmt)
  {
    const_stmt->value->accept(this);

    auto result = std::move(this->stack.pop());

    this->current_namespace->environment.declare(const_stmt->identifier.lexeme, std::move(result));
  }

  void visit_while_stmt(WhileStmt *while_stmt)
  {
    while_stmt->condition->accept(this);
    auto condition_result = std::move(this->stack.pop());

    auto num_envs = this->current_namespace->environment.envs.size();

    while (as_type<bool>(condition_result))
    {
      try
      {
        while_stmt->stmt->accept(this);
      }
      catch (BreakException e)
      {
        auto previous_size = this->current_namespace->environment.envs.size();
        for (int i = 0; i < previous_size - num_envs; i++)
        {
          this->current_namespace->environment.pop_env();
        }

        break;
      }
      catch (ContinueException e)
      {
        auto previous_size = this->current_namespace->environment.envs.size();
        for (int i = 0; i < previous_size - num_envs; i++)
        {
          this->current_namespace->environment.pop_env();
        }

        while_stmt->condition->accept(this);
        condition_result = std::move(this->stack.pop());
        continue;
      }

      while_stmt->condition->accept(this);
      condition_result = std::move(this->stack.pop());
    }
  }

  void visit_for_stmt(ForStmt *for_stmt)
  {
    this->current_namespace->environment.push_env();

    if (for_stmt->initializer.has_value())
    {
      for_stmt->initializer.value()->accept(this);
    }

    auto num_envs = this->current_namespace->environment.envs.size();

    while (true)
    {
      if (for_stmt->condition.has_value())
      {
        for_stmt->condition.value()->accept(this);
        auto condition_result = std::move(this->stack.pop());

        if (!as_type<bool>(condition_result.data))
        {
          break;
        }
      }

      try
      {
        for_stmt->body->accept(this);
      }
      catch (BreakException e)
      {
        auto previous_size = this->current_namespace->environment.envs.size();
        for (int i = 0; i < previous_size - num_envs; i++)
        {
          this->current_namespace->environment.pop_env();
        }

        break;
      }
      catch (ContinueException e)
      {
        auto previous_size = this->current_namespace->environment.envs.size();
        for (int i = 0; i < previous_size - num_envs; i++)
        {
          this->current_namespace->environment.pop_env();
        }

        if (for_stmt->increment.has_value())
        {
          for_stmt->increment.value()->accept(this);
        }
        continue;
      }

      if (for_stmt->increment.has_value())
      {
        for_stmt->increment.value()->accept(this);
      }
    }

    this->current_namespace->environment.pop_env();
  }

  void visit_binary(Binary *binary)
  {
    switch (binary->op.token_type)
    {
    case Token::Type::AND:
    case Token::Type::OR:
      visit_binary_short_circuit(binary);
      break;
    default:
      visit_binary_normal(binary);
    }
  }

  void visit_binary_short_circuit(Binary *binary)
  {
    binary->left->accept(this);
    auto left = std::move(this->stack.pop());

    switch (binary->op.token_type)
    {
    case Token::Type::AND:
    {
      if (!is_type<bool>(left))
      {
        throw BirdException("The 'and' binary operator could not be used to "
                            "interpret these values.");
      }
      if (as_type<bool>(left) == false)
      {
        this->stack.push(Value(false));
      }
      else
      {
        binary->right->accept(this);
      }
      break;
    }
    case Token::Type::OR:
    {
      if (!is_type<bool>(left))
      {
        throw BirdException("The 'or' binary operator could not be used to "
                            "interpret these values.");
      }
      if (as_type<bool>(left) == true)
      {
        this->stack.push(Value(true));
      }
      else
      {
        binary->right->accept(this);
      }
      break;
    }
    default:
    {
      throw std::runtime_error("Undefined binary short-circuit operator.");
    }
    }
  }

  void visit_binary_normal(Binary *binary)
  {
    binary->left->accept(this);
    binary->right->accept(this);

    auto right = std::move(this->stack.pop());
    auto left = std::move(this->stack.pop());

    switch (binary->op.token_type)
    {
    case Token::Type::XOR:
    {
      this->stack.push(left != right);
      break;
    }
    case Token::Type::PLUS:
    {
      this->stack.push(left + right);
      break;
    }
    case Token::Type::MINUS:
    {
      this->stack.push(left - right);
      break;
    }
    case Token::Type::SLASH:
    {
      this->stack.push(left / right);
      break;
    }
    case Token::Type::STAR:
    {
      this->stack.push(left * right);
      break;
    }
    case Token::Type::GREATER:
    {
      this->stack.push(left > right);
      break;
    }
    case Token::Type::GREATER_EQUAL:
    {
      this->stack.push(left >= right);
      break;
    }
    case Token::Type::LESS:
    {
      this->stack.push(left < right);
      break;
    }
    case Token::Type::LESS_EQUAL:
    {
      this->stack.push(left <= right);
      break;
    }
    case Token::Type::BANG_EQUAL:
    {
      this->stack.push(left != right);
      break;
    }
    case Token::Type::EQUAL_EQUAL:
    {
      this->stack.push(left == right);
      break;
    }
    case Token::Type::PERCENT:
    {
      this->stack.push(left % right);
      break;
    }
    default:
    {
      throw std::runtime_error("Undefined binary operator.");
    }
    }
  }

  void visit_unary(Unary *unary)
  {
    unary->expr->accept(this);
    auto expr = std::move(this->stack.pop());

    switch (unary->op.token_type)
    {
    case Token::Type::MINUS:
    {
      this->stack.push(-expr);
      break;
    }
    case Token::Type::QUESTION:
    {
      this->stack.push(expr != Value(nullptr));
      break;
    }
    case Token::Type::NOT:
    {
      this->stack.push(!expr);
      break;
    }
    default:
    {
      throw std::runtime_error("Undefined unary operator.");
    }
    }
  }

  void visit_primary(Primary *primary)
  {
    switch (primary->value.token_type)
    {
    case Token::Type::FLOAT_LITERAL:
      this->stack.push(Value(variant(std::stod(primary->value.lexeme))));
      break;
    case Token::Type::BOOL_LITERAL:
      this->stack.push(
          Value(variant(primary->value.lexeme == "true" ? true : false)));
      break;
    case Token::Type::STR_LITERAL:
      this->stack.push(Value(variant(primary->value.lexeme)));
      break;
    case Token::Type::INT_LITERAL:
      this->stack.push(Value(variant(std::stoi(primary->value.lexeme))));
      break;
    case Token::Type::IDENTIFIER:
      this->stack.push(this->current_namespace->environment.get(primary->value.lexeme));
      break;
    default:
      throw std::runtime_error("undefined primary value");
    }
  }

  void visit_ternary(Ternary *ternary)
  {
    ternary->condition->accept(this);

    auto result = std::move(this->stack.pop());

    if (as_type<bool>(result))
      ternary->true_expr->accept(this);
    else
      ternary->false_expr->accept(this);
  }

  void visit_func(Func *func)
  {
    Callable callable =
        Callable(func->param_list, func->block, func->return_type);

    this->current_namespace->call_table.declare(func->identifier.lexeme, callable);
  }

  void visit_if_stmt(IfStmt *if_stmt)
  {
    if_stmt->condition->accept(this);
    auto result = std::move(this->stack.pop());

    if (as_type<bool>(result))
      if_stmt->then_branch->accept(this);
    else if (if_stmt->else_branch.has_value())
      if_stmt->else_branch.value()->accept(this);
  }

  void visit_call(Call *call)
  {
    auto callable = this->current_namespace->call_table.get(call->identifier.lexeme);
    callable.call(this, call->args);
  }

  void visit_return_stmt(ReturnStmt *return_stmt)
  {
    if (return_stmt->expr.has_value())
    {
      return_stmt->expr.value()->accept(this);
    }

    throw ReturnException();
  }

  void visit_break_stmt(BreakStmt *break_stmt) { throw BreakException(); }

  void visit_continue_stmt(ContinueStmt *continue_stmt)
  {
    throw ContinueException();
  }

  void visit_type_stmt(TypeStmt *type_stmt)
  {
    this->current_namespace->type_table.declare(
        type_stmt->identifier.lexeme,
        this->type_converter.convert(type_stmt->type_token));
  }

  void visit_subscript(Subscript *Subscript)
  {
    Subscript->subscriptable->accept(this);
    auto subscriptable = this->stack.pop();

    Subscript->index->accept(this);
    auto index = this->stack.pop();

    this->stack.push(subscriptable[index]);
  }

  void visit_struct_decl(StructDecl *struct_decl)
  {
    std::vector<std::pair<std::string, std::shared_ptr<BirdType>>>
        struct_fields;
    std::transform(
        struct_decl->fields.begin(), struct_decl->fields.end(),
        std::back_inserter(struct_fields),
        [&](std::pair<std::string, std::shared_ptr<ParseType::Type>> field)
        {
          return std::make_pair(field.first,
                                this->type_converter.convert(field.second));
        });

    this->current_namespace->type_table.declare(
        struct_decl->identifier.lexeme,
        std::make_shared<StructType>(struct_decl->identifier.lexeme,
                                     std::move(struct_fields)));
  }

  void visit_direct_member_access(DirectMemberAccess *direct_member_access)
  {
    direct_member_access->accessable->accept(this);
    auto accessable = this->stack.pop();

    if (is_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(
            accessable))
    {
      auto struct_type =
          as_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(
              accessable);
      this->stack.push(
          (*struct_type.get())[direct_member_access->identifier.lexeme]);
    }
    else
    {
      throw std::runtime_error("Cannot access member of non-struct type.");
    }
  }

  void visit_struct_initialization(StructInitialization *struct_initialization)
  {
    std::shared_ptr<std::unordered_map<std::string, Value>> struct_instance =
        std::make_shared<std::unordered_map<std::string, Value>>();

    auto previous_ns = this->current_namespace;

    while (this->current_namespace && !this->current_namespace->type_table.contains(struct_initialization->identifier.lexeme))
    {
      this->current_namespace = this->current_namespace->parent;
    }

    if (!this->current_namespace)
    {
      throw std::runtime_error("cannot find struct type");
    }

    auto type = this->current_namespace->type_table.get(struct_initialization->identifier.lexeme);

    auto struct_type = safe_dynamic_pointer_cast<StructType>(type);

    for (auto &field : struct_type->fields)
    {
      bool found = false;
      for (auto &field_assignment : struct_initialization->field_assignments)
      {
        if (field.first == field_assignment.first)
        {
          found = true;
          field_assignment.second->accept(this);
          auto result = this->stack.pop();

          (*struct_instance)[field_assignment.first] = result;
          break;
        }
      }

      if (!found)
      {
        if (field.second->type == BirdTypeType::BOOL)
        {
          (*struct_instance)[field.first] = Value(false);
        }
        else if (field.second->type == BirdTypeType::INT)
        {
          (*struct_instance)[field.first] = Value(0);
        }
        else if (field.second->type == BirdTypeType::FLOAT)
        {
          (*struct_instance)[field.first] = Value(0.0);
        }
        else if (field.second->type == BirdTypeType::STRING)
        {
          (*struct_instance)[field.first] = Value("");
        }
        else if (field.second->type == BirdTypeType::STRUCT)
        {
          (*struct_instance)[field.first] = Value(nullptr);
        }
        else if (field.second->type == BirdTypeType::PLACEHOLDER)
        {
          (*struct_instance)[field.first] = Value(nullptr);
        }
        else
        {
          throw std::runtime_error("Cannot assign member of non-struct type.");
        }
      }
    }

    this->current_namespace = previous_ns;

    this->stack.push(Value(struct_instance));
  }

  void visit_member_assign(MemberAssign *member_assign)
  {
    member_assign->accessable->accept(this);
    auto accessable = this->stack.pop();

    if (is_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(
            accessable))
    {
      auto struct_type =
          as_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(
              accessable);

      member_assign->value->accept(this);
      auto value = this->stack.pop();

      (*struct_type.get())[member_assign->identifier.lexeme] = value;
    }
    else
    {
      throw std::runtime_error("Cannot assign member of non-struct type.");
    }
  }

  void visit_as_cast(AsCast *as_cast)
  {
    as_cast->expr->accept(this);
    auto expr = this->stack.pop();

    if (as_cast->type->tag == ParseType::PRIMITIVE)
    {
      auto primitive =
          safe_dynamic_pointer_cast<ParseType::Primitive, ParseType::Type>(
              as_cast->type);
      auto token = primitive->type;
      if (token.lexeme == "int" && is_type<double>(expr))
      {
        this->stack.push(Value((int)as_type<double>(expr)));
        return;
      }

      if (token.lexeme == "float" && is_type<int>(expr))
      {
        this->stack.push(Value((double)as_type<int>(expr)));
        return;
      }
    }

    this->stack.push(expr);
  }

  void visit_array_init(ArrayInit *array_init)
  {
    std::vector<Value> elements;

    for (const auto &element : array_init->elements)
    {
      element->accept(this);
      elements.push_back(this->stack.pop());
    }

    this->stack.push(Value(std::make_shared<std::vector<Value>>(elements)));
  }

  void visit_index_assign(IndexAssign *index_assign)
  {
    index_assign->lhs->subscriptable->accept(this);
    auto lhs = this->stack.pop();

    index_assign->lhs->index->accept(this);
    auto index = this->stack.pop();

    index_assign->rhs->accept(this);
    auto rhs = this->stack.pop();

    if (is_type<std::shared_ptr<std::vector<Value>>>(lhs))
    {
      auto arr = as_type<std::shared_ptr<std::vector<Value>>>(lhs);
      int idx = as_type<int>(index);

      (*arr)[idx] = rhs;
    }
    else
    {
      throw BirdException("expected array");
    }
  }

  void visit_match_expr(MatchExpr *match_expr)
  {
    match_expr->expr->accept(this);
    auto expr = this->stack.pop();

    for (auto &arm : match_expr->arms)
    {
      arm.first->accept(this);
      auto pattern = this->stack.pop();

      if (as_type<bool>((expr == pattern).data))
      {
        arm.second->accept(this);
        return;
      }
    }

    match_expr->else_arm->accept(this);
  }

  void visit_namespace(NamespaceStmt *_namespace)
  {
    auto name = _namespace->identifier.lexeme;

    auto previous_namespace = this->current_namespace;

    if (this->current_namespace->nested_namespaces.find(name) ==
        this->current_namespace->nested_namespaces.end())
    {
      auto new_ns = std::make_shared<Namespace<Value, Callable, std::shared_ptr<BirdType>>>(this->current_namespace);
      this->current_namespace->nested_namespaces[name] = new_ns;
    }

    this->current_namespace = this->current_namespace->nested_namespaces[name];

    for (auto &member : _namespace->members)
    {
      member->accept(this);
    }

    this->current_namespace = previous_namespace;
  }

  void visit_scope_resolution(ScopeResolutionExpr *scope_resolution)
  {
    auto name = scope_resolution->_namespace.lexeme;
    auto previous_namespace = this->current_namespace;

    while (this->current_namespace &&
           this->current_namespace->nested_namespaces.find(name) == this->current_namespace->nested_namespaces.end())
    {
      this->current_namespace = this->current_namespace->parent;
    }

    if (!this->current_namespace)
    {
      throw std::runtime_error("undefined namespace: " + name);
    }

    this->current_namespace = this->current_namespace->nested_namespaces[name];

    scope_resolution->identifier->accept(this);

    this->current_namespace = previous_namespace;
  }
};