#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_const_stmt(ConstStmt *const_stmt) {
  const_stmt->value->accept(this);
  TaggedExpression initializer = this->stack.pop();

  std::shared_ptr<BirdType> type = initializer.type;

  BinaryenIndex index =
      this->function_locals[this->current_function_name].size();
  this->function_locals[this->current_function_name].push_back(
      bird_type_to_binaryen_type(type));

  environment.declare(const_stmt->identifier.lexeme, TaggedIndex(index, type));

  BinaryenExpressionRef set_local =
      this->binaryen_set(const_stmt->identifier.lexeme, initializer.value);

  this->stack.push(TaggedExpression(set_local, type));
}
