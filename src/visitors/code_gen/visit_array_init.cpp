#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

void CodeGen::visit_array_init(ArrayInit *array_init) {
  std::vector<BinaryenExpressionRef> vals;

  unsigned int mem_size = 0;
  std::shared_ptr<BirdType> type =
      std::make_shared<IntType>(); // do not touch this: this will not work when
                                   // set to void type
  for (auto &element : array_init->elements) {
    element->accept(this);
    auto val = this->stack.pop();
    type = val.type;

    vals.push_back(val.value);
    mem_size += bird_type_byte_size(val.type);
  }

  auto &locals = this->function_locals[this->current_function_name];
  locals.push_back(BinaryenTypeInt32());

  auto identifier = std::to_string(locals.size() - 1) + "temp";
  this->environment.declare(
      identifier,
      TaggedIndex(locals.size() - 1, std::make_shared<ArrayType>(type)));

  std::vector<BinaryenExpressionRef> args = {
      BinaryenConst(this->mod, BinaryenLiteralInt32(mem_size)),
      type_is_on_heap(type->get_tag())
          ? BinaryenConst(this->mod,
                          BinaryenLiteralInt32(array_init->elements.size()))
          : BinaryenConst(this->mod, BinaryenLiteralInt32(0))};

  TaggedExpression local_set = this->binaryen_set(
      identifier, BinaryenCall(this->mod, "mem_alloc", args.data(), args.size(),
                               BinaryenTypeInt32()));

  std::vector<BinaryenExpressionRef> binaryen_calls = {local_set.value};

  unsigned int offset = 0;
  for (auto val : vals) {
    BinaryenExpressionRef args[3] = {
        this->binaryen_get(identifier).value,
        BinaryenConst(this->mod, BinaryenLiteralInt32(offset)), val};
    binaryen_calls.push_back(BinaryenCall(this->mod,
                                          get_mem_set_for_type(type->get_tag()),
                                          args, 3, BinaryenTypeNone()));
    offset += bird_type_byte_size(type);
  }

  std::vector<BinaryenExpressionRef> array_struct_args = {
      this->binaryen_get(identifier).value,
      BinaryenConst(this->mod,
                    BinaryenLiteralInt32(array_init->elements.size()))};

  auto call = BinaryenCall(this->mod, struct_constructors["0array"].c_str(),
                           array_struct_args.data(), array_struct_args.size(),
                           BinaryenTypeInt32());
  auto create_ref = BinaryenCall(this->mod, struct_constructors["0ref"].c_str(),
                                 &call, 1, BinaryenTypeInt32());
  binaryen_calls.push_back(this->binaryen_set(identifier, create_ref).value);
  auto get_ref = this->binaryen_get(identifier);
  auto register_root = BinaryenCall(this->mod, "register_root", &get_ref.value,
                                    1, BinaryenTypeNone());

  binaryen_calls.push_back(register_root);
  binaryen_calls.push_back(this->binaryen_get(identifier).value);
  auto block = BinaryenBlock(this->mod, nullptr, binaryen_calls.data(),
                             binaryen_calls.size(), BinaryenTypeInt32());

  this->stack.push(TaggedExpression(block, std::make_shared<ArrayType>(type)));
}

BinaryenExpressionRef CodeGen::deref(BinaryenExpressionRef &ref) {
  std::vector<BinaryenExpressionRef> operands = {
      ref, BinaryenConst(this->mod, BinaryenLiteralInt32(0))};
  return BinaryenCall(this->mod, "mem_get_32", operands.data(), operands.size(),
                      BinaryenTypeInt32());
}