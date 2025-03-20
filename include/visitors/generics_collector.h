
#pragma once
#include "visitor_adapter.h"
#include <memory>
#include <unordered_map>

class GenericsCollector : public VisitorAdapter {
public:
  GenericsCollector() {};
  std::unordered_map<std::string,
                     std::vector<std::vector<std::shared_ptr<ParseType::Type>>>>
      name_to_type_args;
  std::unordered_map<std::string,
                     std::vector<std::vector<std::shared_ptr<ParseType::Type>>>>
  collect_types(std::vector<std::unique_ptr<Stmt>> *stmts) {
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }

    return name_to_type_args;
  }

  void visit_func(Func *func) override {
    func->block->accept(this);
    if (func->get_type()->get_tag() != TypeTag::GENERIC_FUNCTION) {
      return;
    }

    auto generic_func_type =
        std::dynamic_pointer_cast<GenericFunction>(func->get_type());

    name_to_type_args[generic_func_type->identifier.lexeme] =
        std::vector<std::vector<std::shared_ptr<ParseType::Type>>>{};
  }

  void visit_call(Call *call) override {
    call->callable->accept(this);
    if (call->callable->get_type()->get_tag() != TypeTag::GENERIC_FUNCTION) {
      return;
    }

    auto generic_function =
        std::dynamic_pointer_cast<GenericFunction>(call->callable->get_type());

    this->name_to_type_args[generic_function->identifier.lexeme].push_back(
        call->type_args);

    for (auto arg : call->args) {
      arg->accept(this);
    }
  }

  void visit_primary(Primary *primary) override {
    if (primary->value.token_type != Token::Type::IDENTIFIER) {
      return;
    }

    if (this->name_to_type_args.count(primary->value.lexeme) == 0) {
      return;
    }

    auto old_token = primary->value;
    auto implementation_count =
        this->name_to_type_args.at(primary->value.lexeme).size();
    auto new_token =
        Token(old_token.token_type,
              std::to_string(implementation_count) + old_token.lexeme,
              old_token.line_num, old_token.char_num);

    primary->value = new_token;
  }
};