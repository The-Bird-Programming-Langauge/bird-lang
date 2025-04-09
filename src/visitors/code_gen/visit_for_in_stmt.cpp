#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <vector>

// TODO: get rid of magic numbers and accound for break and continue
void CodeGen::visit_for_in_stmt(ForInStmt *for_in) {
  environment.push_env();

  for_in->iterable->accept(this);

  auto iterable = stack.pop();

  auto iter_type = safe_dynamic_pointer_cast<IteratorType>(iterable.type);

  auto array_type =
      safe_dynamic_pointer_cast<ArrayType>(iter_type->element_type);

  auto type = array_type->element_type;

  auto binaryen_type = bird_type_to_binaryen_type(type);

  auto iter_local = function_locals[this->current_function_name].size();
  function_locals[this->current_function_name].push_back(BinaryenTypeInt32());

  // set local for iterator
  std::vector<BinaryenExpressionRef> body_block;
  auto set_iter_local = BinaryenLocalSet(this->mod, iter_local, iterable.value);
  body_block.push_back(set_iter_local);

  auto iter = BinaryenLocalGet(this->mod, iter_local, BinaryenTypeInt32());

  // get pointer to arrray in iter at offset 0
  std::vector<BinaryenExpressionRef> mem_get_ref{
      iter, BinaryenConst(this->mod, BinaryenLiteralInt32(0))};

  auto get_ref = BinaryenCall(this->mod, "mem_get_32", mem_get_ref.data(),
                              mem_get_ref.size(), BinaryenTypeInt32());

  // get length of array in iter at offset 4
  std::vector<BinaryenExpressionRef> mem_get_length{
      iter, BinaryenConst(this->mod, BinaryenLiteralInt32(4))};

  auto get_length = BinaryenCall(this->mod, "mem_get_32", mem_get_length.data(),
                                 mem_get_length.size(), BinaryenTypeInt32());

  // get current index in iter at index 8
  std::vector<BinaryenExpressionRef> mem_get_idx{
      iter, BinaryenConst(this->mod, BinaryenLiteralInt32(8))};

  auto get_index = BinaryenCall(this->mod, "mem_get_32", mem_get_idx.data(),
                                mem_get_idx.size(), BinaryenTypeInt32());

  // generate for loop condition with index and length
  auto condition =
      BinaryenBinary(this->mod, BinaryenLtUInt32(), get_index, get_length);

  auto offset = BinaryenBinary(
      this->mod, BinaryenMulInt32(), get_index,
      BinaryenConst(this->mod,
                    BinaryenLiteralInt32(bird_type_byte_size(type))));

  std::vector<BinaryenExpressionRef> args{get_ref, offset};

  auto value = BinaryenCall(this->mod, get_mem_get_for_type(type->get_tag()),
                            args.data(), args.size(), binaryen_type);

  // declare identifier
  auto var_index = function_locals[this->current_function_name].size();
  function_locals[this->current_function_name].push_back(binaryen_type);

  std::vector<BinaryenExpressionRef> loop_body;

  this->environment.declare(for_in->identifier.lexeme,
                            TaggedIndex(var_index, type));

  // bind to identifier
  auto set_loop_var = this->binaryen_set(for_in->identifier.lexeme, value);

  loop_body.push_back(set_loop_var.value);

  for_in->body->accept(this);

  auto body = stack.pop().value;

  loop_body.push_back(body);

  // increment the index by 1
  std::vector<BinaryenExpressionRef> increment_args{
      iter, BinaryenConst(this->mod, BinaryenLiteralInt32(8)),
      BinaryenBinary(this->mod, BinaryenAddInt32(), get_index,
                     BinaryenConst(this->mod, BinaryenLiteralInt32(1)))};

  auto increment = BinaryenCall(this->mod, "mem_set_32", increment_args.data(),
                                increment_args.size(), BinaryenTypeNone());

  loop_body.push_back(increment);

  loop_body.push_back(BinaryenBreak(this->mod, "loop", condition, nullptr));

  auto loop_block = BinaryenBlock(this->mod, nullptr, loop_body.data(),
                                  loop_body.size(), BinaryenTypeNone());

  auto loop = BinaryenLoop(
      this->mod, "loop", BinaryenIf(this->mod, condition, loop_block, nullptr));

  body_block.push_back(loop);

  stack.push(BinaryenBlock(this->mod, nullptr, body_block.data(),
                           body_block.size(), BinaryenTypeNone()));

  environment.pop_env();
}
