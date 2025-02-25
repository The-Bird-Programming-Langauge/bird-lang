#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_array_init(ArrayInit *array_init)
{
  std::vector<BinaryenExpressionRef> vals;

  // initilize to 5 for the length int
  unsigned int mem_size = 5;
  std::shared_ptr<BirdType> type;
  for (auto &element : array_init->elements)
  {
    element->accept(this);
    auto val = this->stack.pop();
    type = val.type;

    vals.push_back(val.value);
    mem_size += bird_type_byte_size(val.type);
  }

  auto &locals = this->function_locals[this->current_function_name];
  locals.push_back(BinaryenTypeInt32());

  auto identifier = std::to_string(locals.size()) + "temp";
  this->environment.declare(identifier, TaggedIndex(locals.size(), type));

  auto mem_size_literal = BinaryenConst(this->mod, BinaryenLiteralInt32(mem_size));
  BinaryenExpressionRef local_set = this->binaryen_set(
      identifier,
      BinaryenCall(this->mod,
                   "mem_alloc",
                   &mem_size_literal,
                   1,
                   BinaryenTypeInt32()));

  std::vector<BinaryenExpressionRef> binaryen_calls = {local_set};

  unsigned int offset = 0;

  BinaryenExpressionRef length_args[3] = {
      this->binaryen_get(identifier),
      BinaryenConst(this->mod, BinaryenLiteralInt32(offset)),
      BinaryenConst(this->mod, BinaryenLiteralInt32(array_init->elements.size()))};

  binaryen_calls.push_back(BinaryenCall(
      this->mod,
      "mem_set_32",
      length_args,
      3,
      BinaryenTypeNone()));

  offset += 5;

  for (auto val : vals)
  {
    BinaryenExpressionRef args[3] = {
        this->binaryen_get(identifier),
        BinaryenConst(this->mod, BinaryenLiteralInt32(offset)),
        val};

    binaryen_calls.push_back(BinaryenCall(
        this->mod,
        type->type == BirdTypeType::FLOAT ? "mem_set_64" : "mem_set_32",
        args,
        3,
        BinaryenTypeNone()));

    offset += bird_type_byte_size(type);
  }

  binaryen_calls.push_back(this->binaryen_get(identifier));

  auto block = BinaryenBlock(this->mod,
                             nullptr,
                             binaryen_calls.data(),
                             binaryen_calls.size(),
                             BinaryenTypeInt32());

  this->stack.push(TaggedExpression(block, std::make_shared<ArrayType>(type)));
}