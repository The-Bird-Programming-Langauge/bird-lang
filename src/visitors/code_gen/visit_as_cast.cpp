#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_as_cast(AsCast *as_cast) {
  as_cast->expr->accept(this);
  auto expr = this->stack.pop();

  std::shared_ptr<BirdType> to_type =
      this->type_converter.convert(as_cast->type);

  // if (to_type->get_tag() == TypeTag::INT &&
  //     expr.type->get_tag() == TypeTag::FLOAT) {
  //   this->stack.push(TaggedExpression(
  //       BinaryenUnary(this->mod, BinaryenTruncSatSFloat64ToInt32(),
  //       expr.value), std::shared_ptr<BirdType>(new IntType())));
  //   return;
  // } else if (to_type->get_tag() == TypeTag::FLOAT &&
  //            expr.type->get_tag() == TypeTag::INT) {
  //   this->stack.push(TaggedExpression(
  //       BinaryenUnary(this->mod, BinaryenConvertSInt32ToFloat64(),
  //       expr.value), std::shared_ptr<BirdType>(new FloatType())));
  //   return;
  // }

  if (expr.type->get_tag() == to_type->get_tag()) {
    this->stack.push(expr);
    return;
  }

  try {
    auto binary_op = this->binary_operations.at(Token::Type::AS);
    auto binary_op_fn =
        binary_op.at({expr.type->get_tag(), to_type->get_tag()});

    this->stack.push(TaggedExpression(
        BinaryenUnary(this->mod, binary_op_fn.value(), expr.value), to_type));

  } catch (std::out_of_range &e) {
    throw BirdException("unsupported cast from " + expr.type->to_string() +
                        " to " + to_type->to_string());
  }
}