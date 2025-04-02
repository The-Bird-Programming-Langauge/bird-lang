#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_break_stmt(BreakStmt *break_stmt) {
  std::vector<BinaryenExpressionRef> children{};
  for (auto &[string, env_index] : this->environment.envs.back()) {
    auto get_result = this->binaryen_get(string);
    if (type_is_on_heap(get_result.type->get_tag())) {
      auto unregister = BinaryenCall(this->mod, "unregister_root",
                                     &get_result.value, 1, BinaryenTypeNone());
      children.push_back(unregister);
    }
  }
  children.push_back(BinaryenBreak(this->mod, "EXIT", nullptr, nullptr));

  auto block = BinaryenBlock(this->mod, nullptr, children.data(),
                             children.size(), BinaryenTypeNone());

  this->stack.push(TaggedExpression(block, std::make_shared<VoidType>()));
}