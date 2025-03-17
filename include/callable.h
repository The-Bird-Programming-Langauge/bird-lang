#pragma once

#include "bird_type.h"
#include "core_call_table.h"
#include "parse_type.h"
#include "token.h"
#include <memory>
#include <optional>
#include <vector>

class Stmt;
class Expr;
class Interpreter;
class Value;

class Callable {
public:
  std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>> param_list;
  std::shared_ptr<Stmt> block;
  std::optional<std::shared_ptr<ParseType::Type>> return_type;

  virtual ~Callable() {}
  Callable(std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>>
               param_list,
           std::shared_ptr<Stmt> block,
           std::optional<std::shared_ptr<ParseType::Type>> return_type)
      : param_list(param_list), block(std::move(block)),
        return_type(return_type) {}
  Callable() = default;
  Callable(const Callable &other)
      : param_list(other.param_list), block(std::move(other.block)),
        return_type(other.return_type) {}

  virtual void call(Interpreter *Interpreter, std::vector<Value> args);
};

class Length : public Callable {
public:
  Length() : Callable() {}
  void call(Interpreter *interpreter, std::vector<Value> args) override;
};