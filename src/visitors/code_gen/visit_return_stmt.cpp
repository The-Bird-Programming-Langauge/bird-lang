#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_return_stmt(ReturnStmt *return_stmt) {
  TaggedType func_return_type =
      this->function_return_types[this->current_function_name];

  std::vector<BinaryenExpressionRef> children{};
  for (auto &[string, env_index] : this->environment.envs.back()) {
    auto get_result = this->binaryen_get(string);
    if (env_index.value >=
            this->function_param_count[this->current_function_name] &&
        type_is_on_heap(get_result.type->get_tag())) {
      auto unregister = BinaryenCall(this->mod, "unregister_root",
                                     &get_result.value, 1, BinaryenTypeNone());
      children.push_back(unregister);
    }
  }

  auto block =
      BinaryenBlock(this->mod, nullptr, children.data(), children.size(),
                    bird_type_to_binaryen_type(func_return_type.type));

  if (return_stmt->expr.has_value()) {
    return_stmt->expr.value()->accept(this);
    auto result = this->stack.pop();

    if (*result.type != *func_return_type.type) {
      throw BirdException("return type mismatch");
    }
    BinaryenBlockInsertChildAt(block, children.size(), result.value);
    this->stack.push(TaggedExpression(BinaryenReturn(this->mod, block),
                                      func_return_type.type));
  } else {
    BinaryenBlockInsertChildAt(block, children.size(),
                               BinaryenReturn(this->mod, nullptr));
    this->stack.push(TaggedExpression(block, std::make_shared<VoidType>()));
  }
}
