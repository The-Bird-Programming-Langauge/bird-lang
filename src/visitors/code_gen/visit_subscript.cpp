#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_subscript(Subscript *subscript) {
  subscript->subscriptable->accept(this);
  auto subscriptable = this->stack.pop();

  subscript->index->accept(this);
  auto index = this->stack.pop();

  std::shared_ptr<BirdType> type;
  if (subscriptable.type->type == BirdTypeType::ARRAY) {
    type =
        safe_dynamic_pointer_cast<ArrayType>(subscriptable.type)->element_type;
  }

  // TODO: make this better
  BinaryenExpressionRef args[2] = {
      subscriptable.value,
      subscriptable.type->type == BirdTypeType::ARRAY
          ? BinaryenBinary(
                this->mod, BinaryenMulInt32(), index.value,
                BinaryenConst(this->mod,
                              BinaryenLiteralInt32(bird_type_byte_size(type))))
          : index.value};

  this->stack.push(TaggedExpression(
      BinaryenCall(this->mod,
                   type->type == BirdTypeType::FLOAT ? "mem_get_64"
                                                     : "mem_get_32",
                   args, 2, BinaryenTypeInt32()),
      std::shared_ptr<BirdType>(type)));
}