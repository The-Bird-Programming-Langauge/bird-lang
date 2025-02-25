#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_subscript(Subscript *subscript) {
  subscript->subscriptable->accept(this);
  auto subscriptable = this->stack.pop();

  if (subscriptable.type->type != BirdTypeType::ARRAY) {
    throw BirdException("Tried to subscript into non-array type");
  }

  subscript->index->accept(this);
  auto index = this->stack.pop();

  std::shared_ptr<BirdType> type =
      safe_dynamic_pointer_cast<ArrayType>(subscriptable.type)->element_type;

  // TODO: make this better
  BinaryenExpressionRef args[2] = {
      subscriptable.value,
      BinaryenBinary(
          this->mod, BinaryenAddInt32(),
          BinaryenBinary(
              this->mod, BinaryenMulInt32(), index.value,
              BinaryenConst(this->mod,
                            BinaryenLiteralInt32(bird_type_byte_size(type)))),
          BinaryenConst(this->mod, BinaryenLiteralInt32(5)))};

  this->stack.push(TaggedExpression(
      BinaryenCall(this->mod,
                   type->type == BirdTypeType::FLOAT ? "mem_get_64"
                                                     : "mem_get_32",
                   args, 2, BinaryenTypeInt32()),
      std::shared_ptr<BirdType>(type)));
}