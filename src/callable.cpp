#include "../include/callable.h"
#include "../include/sym_table.h"
#include "../include/value.h"
#include "../include/visitors/interpreter.h"

void Callable::call(Interpreter *interpreter, std::vector<Value> args) {
  auto original_env = interpreter->env.envs.size();

  interpreter->env.push_env();

  for (int i = 0; i < this->param_list.size(); i++) {
    interpreter->env.declare(param_list[i].first.lexeme, args[i]);
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