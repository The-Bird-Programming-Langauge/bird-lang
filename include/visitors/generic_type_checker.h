#pragma once

#include "type_checker.h"
#include "visitor_adapter.h"
#include <memory>
class GenericTypeChecker : public VisitorAdapter {
  Environment<std::shared_ptr<BirdType>> env;
  TypeChecker &type_checker;
  Environment<Func *> funcs;

public:
  GenericTypeChecker(TypeChecker &type_checker) : type_checker(type_checker) {
    funcs.push_env();
  }

  void check_generic_types(std::vector<std::unique_ptr<Stmt>> *stmts) {
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }
  }

  void visit_func(Func *func) {
    if (func->get_type()->get_tag() != TypeTag::GENERIC_FUNCTION) {
      return;
    }

    auto generic_function_type =
        std::dynamic_pointer_cast<GenericFunction>(func->get_type());
    auto bird_function = std::dynamic_pointer_cast<BirdFunction>(
        generic_function_type->function);

    auto new_bird_function = type_checker.create_func(func);

    auto before_error_size =
        type_checker.user_error_tracker.get_errors().size();

    type_checker.visit_func_helper(func, new_bird_function);

    auto after_error_size = type_checker.user_error_tracker.get_errors().size();

    if (after_error_size > before_error_size) {
      type_checker.user_error_tracker.type_error(
          "Invalid type arguments in function " +
              generic_function_type->identifier.lexeme,
          func->identifier);
    }
  }

  void visit_call(Call *call) {
    call->callable->accept(this);
    if (call->callable->get_type()->get_tag() != TypeTag::GENERIC_FUNCTION) {
      return;
    }
    for (auto arg : call->args) {
      arg->accept(this);
    }
  }
};