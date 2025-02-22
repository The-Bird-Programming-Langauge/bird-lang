#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_return_stmt(ReturnStmt *return_stmt) {
  TaggedType func_return_type =
      this->function_return_types[this->current_function_name];

  if (return_stmt->expr.has_value()) {
    return_stmt->expr.value()->accept(this);
    auto result = this->stack.pop();

    if (*result.type != *func_return_type.type) {
      throw BirdException("return type mismatch");
    }

    this->stack.push(TaggedExpression(BinaryenReturn(this->mod, result.value),
                                      func_return_type.type));
  } else {
    this->stack.push(TaggedExpression(BinaryenReturn(this->mod, nullptr),
                                      std::shared_ptr<VoidType>()));
  }
}