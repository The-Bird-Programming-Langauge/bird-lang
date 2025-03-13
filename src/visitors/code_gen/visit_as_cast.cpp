#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_as_cast(AsCast *as_cast) {
  as_cast->expr->accept(this);
  auto expr = this->stack.pop();

  std::shared_ptr<BirdType> to_type =
      this->type_converter.convert(this->type_table, as_cast->type);

  if (to_type->type == BirdTypeType::INT &&
      expr.type->type == BirdTypeType::FLOAT) {
    this->stack.push(TaggedExpression(
        BinaryenUnary(this->mod, BinaryenTruncSatSFloat64ToInt32(), expr.value),
        std::shared_ptr<BirdType>(new IntType())));
    return;
  } else if (to_type->type == BirdTypeType::FLOAT &&
             expr.type->type == BirdTypeType::INT) {
    this->stack.push(TaggedExpression(
        BinaryenUnary(this->mod, BinaryenConvertSInt32ToFloat64(), expr.value),
        std::shared_ptr<BirdType>(new FloatType())));
    return;
  }

  this->stack.push(expr);
}