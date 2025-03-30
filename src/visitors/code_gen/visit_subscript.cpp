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

  subscript->index->accept(this);
  auto index = this->stack.pop();

  std::shared_ptr<BirdType> type;
  if (subscriptable.type->get_tag() == TypeTag::ARRAY) {
    type =
        safe_dynamic_pointer_cast<ArrayType>(subscriptable.type)->element_type;
  } else if (subscriptable.type->get_tag() == TypeTag::STRING) {
    this->subscript_string(subscriptable, index);
    return;
  } else {
    throw BirdException("Cannot subscript into non array or string type");
  }

  auto array_length = BinaryenCall(this->mod, "length", &subscriptable.value, 1,
                                   BinaryenTypeInt32());

  BinaryenExpressionRef bounds_checked_access = BinaryenIf(
      this->mod,
      BinaryenBinary(this->mod, BinaryenLtSInt32(), index.value, array_length),
      get_subscript_result(subscriptable, index, type),
      BinaryenThrow(this->mod, "RuntimeBirdError", nullptr, 0));

  this->stack.push(TaggedExpression(bounds_checked_access, type));
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

  return BinaryenCall(
      mod, type->get_tag() == TypeTag::FLOAT ? "mem_get_64" : "mem_get_32",
      mem_get_args, 2, bird_type_to_binaryen_type(type));
}

void CodeGen::subscript_string(TaggedExpression subscriptable,
                               TaggedExpression index) {
  auto array_length = BinaryenCall(this->mod, "length", &subscriptable.value, 1,
                                   BinaryenTypeInt32());
  BinaryenExpressionRef mem_position = BinaryenBinary(
      mod, BinaryenMulInt32(), index.value,
      BinaryenConst(mod, BinaryenLiteralInt32(bird_type_byte_size(
                             std::make_shared<IntType>()))));

  BinaryenExpressionRef mem_get_args[2] = {get_array_data(subscriptable),
                                           mem_position};

  BinaryenExpressionRef bounds_checked_access = BinaryenIf(
      this->mod,
      BinaryenBinary(this->mod, BinaryenLtSInt32(), index.value, array_length),
      this
          ->generate_string_from_chars({BinaryenCall(
              mod, "mem_get_32", mem_get_args, 2,
              bird_type_to_binaryen_type(std::make_shared<IntType>()))})
          .value,
      BinaryenThrow(this->mod, "RuntimeBirdError", nullptr, 0));

  this->stack.push(
      TaggedExpression(bounds_checked_access, std::make_shared<StringType>()));
}