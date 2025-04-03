#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_decl_stmt(DeclStmt *decl_stmt) {
  decl_stmt->value->accept(this);
  TaggedExpression initializer_value = this->stack.pop();

  std::shared_ptr<BirdType> type = initializer_value.type;

  BinaryenIndex index =
      this->function_locals[this->current_function_name].size();
  this->function_locals[this->current_function_name].push_back(
      bird_type_to_binaryen_type(type));

  environment.declare(decl_stmt->identifier.lexeme, TaggedIndex(index, type));

  BinaryenExpressionRef set_local =
      this->binaryen_set(decl_stmt->identifier.lexeme, initializer_value.value);

  this->stack.push(TaggedExpression(set_local, type));
}