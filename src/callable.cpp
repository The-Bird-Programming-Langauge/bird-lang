#include "../include/callable.h"
#include "../include/exceptions/bird_exception.h"
#include "../include/sym_table.h"
#include "../include/value.h"
#include "../include/visitors/interpreter.h"

void Callable::call(Interpreter *interpreter,
                    std::vector<std::shared_ptr<Expr>> args) {
  // create new scope
  if (args.size() != this->param_list.size()) {
    throw BirdException(
        "Mismatched arguments, expected: " + std::to_string(param_list.size()) +
        ", found: " + std::to_string(args.size()));
  }

  std::vector<Value> evaluated_args;
  for (auto &arg : args) {
    arg->accept(interpreter);
    auto value = interpreter->stack.pop();
    evaluated_args.push_back(value);
  }

  auto original_env = interpreter->env.envs.size();

  interpreter->env.push_env();

  for (int i = 0; i < this->param_list.size(); i++) {
    interpreter->env.declare(param_list[i].first.lexeme, evaluated_args[i]);
  }

  for (auto &stmt : dynamic_cast<Block *>(this->block.get())->stmts) {
    try {
      stmt->accept(interpreter);
    } catch (ReturnException e) {
      while (interpreter->env.envs.size() > original_env) {
        interpreter->env.pop_env();
      }
      return;
    }
  }

  while (interpreter->env.envs.size() > original_env) {
    interpreter->env.pop_env();
  }
}

void Length::call(Interpreter *interpreter,
                  std::vector<std::shared_ptr<Expr>> args) {
  interpreter->run_core_call("length", args);
};