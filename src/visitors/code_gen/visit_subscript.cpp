#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

BinaryenExpressionRef
get_length_binaryen(BinaryenModuleRef mod,
                    Tagged<BinaryenExpressionRef> &subscriptable);

BinaryenExpressionRef get_index_mem_binaryen(
    BinaryenModuleRef mod, Tagged<BinaryenExpressionRef> &subscriptable,
    Tagged<BinaryenExpressionRef> &index, std::shared_ptr<BirdType> type);

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

  auto array_length = get_length_binaryen(this->mod, subscriptable);

  BinaryenExpressionRef bounds_checked_access = BinaryenIf(
      this->mod,
      BinaryenBinary(this->mod, BinaryenLtSInt32(), index.value, array_length),
      get_index_mem_binaryen(this->mod, subscriptable, index, type),
      // BinaryenThrow(this->mod, "Error: index out of bounds", {}, 0));
      BinaryenConst(this->mod, BinaryenLiteralInt32(0)));

  this->stack.push(
      TaggedExpression(bounds_checked_access, std::shared_ptr<BirdType>(type)));
}

BinaryenExpressionRef
get_length_binaryen(BinaryenModuleRef mod,
                    Tagged<BinaryenExpressionRef> &subscriptable) {
  BinaryenExpressionRef mem_get_args[2] = {
      subscriptable.value, BinaryenConst(mod, BinaryenLiteralInt32(0))};
  return BinaryenCall(mod, "mem_get_32", mem_get_args, 2, BinaryenTypeInt32());
}

BinaryenExpressionRef get_index_mem_binaryen(
    BinaryenModuleRef mod, Tagged<BinaryenExpressionRef> &subscriptable,
    Tagged<BinaryenExpressionRef> &index, std::shared_ptr<BirdType> type) {

  BinaryenExpressionRef mem_position = BinaryenBinary(
      mod, BinaryenAddInt32(),
      BinaryenBinary(
          mod, BinaryenMulInt32(), index.value,
          BinaryenConst(mod, BinaryenLiteralInt32(bird_type_byte_size(type)))),
      BinaryenConst(mod, BinaryenLiteralInt32(5)));

  BinaryenExpressionRef mem_get_args[2] = {subscriptable.value, mem_position};
  return BinaryenCall(
      mod, type->type == BirdTypeType::FLOAT ? "mem_get_64" : "mem_get_32",
      mem_get_args, 2, BinaryenTypeInt32());
}