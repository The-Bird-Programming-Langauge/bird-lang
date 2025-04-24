#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_block(Block *block) {
  std::vector<BinaryenExpressionRef> children;
  this->environment.push_env();

  for (auto &stmt : block->stmts) {
    stmt->accept(this);
    auto result = this->stack.pop();

    if (result.value) {
      if (result.type->get_tag() != TypeTag::VOID) {
        children.push_back(BinaryenDrop(this->mod, result.value));
      } else {
        children.push_back(result.value);
      }
    }
  }

  for (auto &[string, env_index] : this->environment.envs.back()) {
    auto get_result = this->binaryen_get(string);
    if (type_is_on_heap(get_result.type->get_tag())) {
      auto unregister = BinaryenCall(this->mod, "unregister_root",
                                     &get_result.value, 1, BinaryenTypeNone());
      children.push_back(unregister);
    }
  }

  this->environment.pop_env();

  BinaryenExpressionRef block_expr = BinaryenBlock(
      this->mod, nullptr, children.data(), children.size(), BinaryenTypeNone());

  this->stack.push(block_expr);
}
