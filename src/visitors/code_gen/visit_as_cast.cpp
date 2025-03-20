#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_as_cast(AsCast *as_cast) {
  as_cast->expr->accept(this);
  auto expr = this->stack.pop();

  std::shared_ptr<BirdType> to_type =
      this->type_converter.convert(as_cast->type);

  if (to_type->get_tag() == TypeTag::INT &&
      expr.type->get_tag() == TypeTag::FLOAT) {
    this->stack.push(TaggedExpression(
        BinaryenUnary(this->mod, BinaryenTruncSatSFloat64ToInt32(), expr.value),
        std::shared_ptr<BirdType>(new IntType())));
    return;
  } else if (to_type->get_tag() == TypeTag::FLOAT &&
             expr.type->get_tag() == TypeTag::INT) {
    this->stack.push(TaggedExpression(
        BinaryenUnary(this->mod, BinaryenConvertSInt32ToFloat64(), expr.value),
        std::shared_ptr<BirdType>(new FloatType())));
    return;
  }

  this->stack.push(expr);
}