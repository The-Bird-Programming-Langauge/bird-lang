#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

BinaryenExpressionRef
get_subscript_result(Tagged<BinaryenExpressionRef> &subscriptable,
                     Tagged<BinaryenExpressionRef> &index,
                     std::shared_ptr<BirdType> type);

void CodeGen::visit_subscript(Subscript *subscript) {
  subscript->subscriptable->accept(this);
  auto subscriptable = this->stack.pop();

  if (subscriptable.type->get_tag() != TypeTag::ARRAY) {
    throw BirdException("Tried to subscript into non-array type");
  }

  subscript->index->accept(this);
  auto index = this->stack.pop();

  std::shared_ptr<BirdType> type =
      safe_dynamic_pointer_cast<ArrayType>(subscriptable.type)->element_type;

  auto array_length = BinaryenCall(this->mod, "length", &subscriptable.value, 1,
                                   BinaryenTypeInt32());

  BinaryenExpressionRef bounds_checked_access = BinaryenIf(
      this->mod,
      BinaryenBinary(this->mod, BinaryenLtSInt32(), index.value, array_length),
      get_subscript_result(subscriptable, index, type),
      // TODO: Get the commented-out line to work and replace the
      // line below with it BinaryenThrow(this->mod, "Error:
      // index out of bounds", {}, 0));
      BinaryenConst(this->mod, type->get_tag() == TypeTag::FLOAT
                                   ? BinaryenLiteralFloat64(0)
                                   : BinaryenLiteralInt32(0)));

  this->stack.push(
      TaggedExpression(bounds_checked_access, std::shared_ptr<BirdType>(type)));
}

BinaryenExpressionRef
CodeGen::get_array_data(Tagged<BinaryenExpressionRef> &subscriptable) {
  BinaryenExpressionRef mem_get_args[2] = {
      subscriptable.value, BinaryenConst(mod, BinaryenLiteralInt32(0))};
  return BinaryenCall(mod, "mem_get_32", mem_get_args, 2, BinaryenTypeInt32());
}

BinaryenExpressionRef
CodeGen::get_subscript_result(Tagged<BinaryenExpressionRef> &subscriptable,
                              Tagged<BinaryenExpressionRef> &index,
                              std::shared_ptr<BirdType> type) {
  BinaryenExpressionRef mem_position = BinaryenBinary(
      mod, BinaryenMulInt32(), index.value,
      BinaryenConst(mod, BinaryenLiteralInt32(bird_type_byte_size(type))));

  BinaryenExpressionRef mem_get_args[2] = {get_array_data(subscriptable),
                                           mem_position};
  return BinaryenCall(mod, get_mem_get_for_type(type->get_tag()), mem_get_args,
                      2, bird_type_to_binaryen_type(type));
}