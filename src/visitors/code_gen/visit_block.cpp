#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_block(Block *block) {
  std::vector<BinaryenExpressionRef> children;
  this->environment.push_env();

  for (auto &stmt : block->stmts) {
    stmt->accept(this);
    auto result = this->stack.pop();

    if (result.value) {
      children.push_back(result.value);
    }

    if (this->must_garbage_collect) {
      this->garbage_collect();
      children.push_back(this->stack.pop().value);
      this->must_garbage_collect = false;
    }
  }

  this->environment.pop_env();

  BinaryenExpressionRef block_expr = BinaryenBlock(
      this->mod, nullptr, children.data(), children.size(), BinaryenTypeNone());

  this->stack.push(block_expr);
}
