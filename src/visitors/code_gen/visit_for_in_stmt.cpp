#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <vector>

#define ITERABLE_OFFSET 0
#define LENGTH_OFFSET 4
#define INCREMENT_OFFSET 8

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

  std::vector<BinaryenExpressionRef> body_block;
  auto set_iter_local = BinaryenLocalSet(this->mod, iter_local, iterable.value);
  body_block.push_back(set_iter_local);

  auto iter = BinaryenLocalGet(this->mod, iter_local, BinaryenTypeInt32());

  std::vector<BinaryenExpressionRef> body_and_children;

  std::vector<BinaryenExpressionRef> mem_get_ref{
      iter, BinaryenConst(this->mod, BinaryenLiteralInt32(ITERABLE_OFFSET))};

  auto get_ref = BinaryenCall(this->mod, "mem_get_32", mem_get_ref.data(),
                              mem_get_ref.size(), BinaryenTypeInt32());

  std::vector<BinaryenExpressionRef> mem_get_length{
      iter, BinaryenConst(this->mod, BinaryenLiteralInt32(LENGTH_OFFSET))};

  auto get_length = BinaryenCall(this->mod, "mem_get_32", mem_get_length.data(),
                                 mem_get_length.size(), BinaryenTypeInt32());

  std::vector<BinaryenExpressionRef> mem_get_idx{
      iter, BinaryenConst(this->mod, BinaryenLiteralInt32(INCREMENT_OFFSET))};

  auto get_index = BinaryenCall(this->mod, "mem_get_32", mem_get_idx.data(),
                                mem_get_idx.size(), BinaryenTypeInt32());

  auto early_exit =
      BinaryenBinary(this->mod, BinaryenGeUInt32(), get_index, get_length);

  body_block.push_back(BinaryenBreak(this->mod, "EXIT", early_exit, nullptr));

  auto offset = BinaryenBinary(
      this->mod, BinaryenMulInt32(), get_index,
      BinaryenConst(this->mod,
                    BinaryenLiteralInt32(bird_type_byte_size(type))));

  std::vector<BinaryenExpressionRef> args{get_ref, offset};

  auto value = BinaryenCall(this->mod, get_mem_get_for_type(type->get_tag()),
                            args.data(), args.size(), binaryen_type);

  auto var_index = function_locals[this->current_function_name].size();
  function_locals[this->current_function_name].push_back(binaryen_type);

  std::vector<BinaryenExpressionRef> loop_body;

  this->environment.declare(for_in->identifier.lexeme,
                            TaggedIndex(var_index, type));

  body_and_children.push_back(
      this->binaryen_set(for_in->identifier.lexeme, value).value);

  for_in->body->accept(this);
  auto block = stack.pop().value;

  body_and_children.push_back(block);

  std::vector<BinaryenExpressionRef> body;

  body.push_back(BinaryenBlock(this->mod, "BODY", body_and_children.data(),
                               body_and_children.size(), BinaryenTypeNone()));

  std::vector<BinaryenExpressionRef> increment_args{
      iter, BinaryenConst(this->mod, BinaryenLiteralInt32(INCREMENT_OFFSET)),
      BinaryenBinary(this->mod, BinaryenAddInt32(), get_index,
                     BinaryenConst(this->mod, BinaryenLiteralInt32(1)))};

  body.push_back(BinaryenCall(this->mod, "mem_set_32", increment_args.data(),
                              increment_args.size(), BinaryenTypeNone()));

  auto condition =
      BinaryenBinary(this->mod, BinaryenLtUInt32(), get_index, get_length);

  body.push_back(BinaryenBreak(this->mod, "LOOP", condition, nullptr));

  auto loop_block = BinaryenBlock(this->mod, "for_body", body.data(),
                                  body.size(), BinaryenTypeNone());

  auto loop = BinaryenLoop(this->mod, "LOOP", loop_block);

  body_block.push_back(loop);

  for (auto &[string, env_index] : this->environment.envs.back()) {
    auto get_result = this->binaryen_get(string);
    if (type_is_on_heap(get_result.type->get_tag())) {
      auto unregister = BinaryenCall(this->mod, "unregister_root",
                                     &get_result.value, 1, BinaryenTypeNone());
      body_block.push_back(unregister);
    }
  }

  stack.push(BinaryenBlock(this->mod, "EXIT", body_block.data(),
                           body_block.size(), BinaryenTypeNone()));

  environment.pop_env();
}
