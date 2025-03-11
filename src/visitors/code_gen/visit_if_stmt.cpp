#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_if_stmt(IfStmt *if_stmt) {
  if_stmt->condition->accept(this);
  auto condition = this->stack.pop();

  if_stmt->then_branch->accept(this);
  auto then_branch = this->stack.pop();

  if (if_stmt->else_branch.has_value()) {
    if_stmt->else_branch.value()->accept(this);
    auto else_branch = this->stack.pop();

    this->stack.push(BinaryenIf(this->mod, condition.value, then_branch.value,
                                else_branch.value));
  } else {
    this->stack.push(
        BinaryenIf(this->mod, condition.value, then_branch.value, nullptr));
  }
}