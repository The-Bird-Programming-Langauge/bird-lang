#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_array_init(ArrayInit *array_init) {
  std::vector<BinaryenExpressionRef> children;
  std::vector<BinaryenExpressionRef> vals;

  unsigned int size = 0;
  std::shared_ptr<BirdType> type;
  for (auto &element : array_init->elements) {
    element->accept(this);
    auto val = this->stack.pop();
    type = val.type;

    vals.push_back(val.value);
    size += bird_type_byte_size(val.type);
  }

  auto locals = this->function_locals[this->current_function_name];
  this->function_locals[this->current_function_name].push_back(
      BinaryenTypeInt32());

  auto identifier = std::to_string(locals.size()) + "temp";
  this->environment.declare(identifier, TaggedIndex(locals.size(), type));

  std::vector<BinaryenExpressionRef> args = {
      BinaryenConst(this->mod, BinaryenLiteralInt32(size)),
      type_is_on_heap(type->type)
          ? BinaryenConst(this->mod,
                          BinaryenLiteralInt32(array_init->elements.size()))
          : BinaryenConst(this->mod, BinaryenLiteralInt32(0))

  };

  BinaryenExpressionRef local_set = this->binaryen_set(
      identifier, BinaryenCall(this->mod, "mem_alloc", args.data(), args.size(),
                               BinaryenTypeInt32()));

  children.push_back(local_set);

  unsigned int offset = 0;
  for (auto val : vals) {
    BinaryenExpressionRef args[3] = {
        this->binaryen_get(identifier),
        BinaryenConst(this->mod, BinaryenLiteralInt32(offset)), val};

    children.push_back(BinaryenCall(this->mod, get_mem_set_for_type(type->type),
                                    args, 3, BinaryenTypeNone()));

    offset += bird_type_byte_size(type);
  }

  children.push_back(this->binaryen_get(identifier));

  auto block = BinaryenBlock(this->mod, nullptr, children.data(),
                             children.size(), BinaryenTypeInt32());

  this->stack.push(TaggedExpression(block, std::make_shared<ArrayType>(type)));
}