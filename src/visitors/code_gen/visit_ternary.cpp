#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_ternary(Ternary *ternary) {
  ternary->condition->accept(this);
  auto condition = this->stack.pop();

  ternary->true_expr->accept(this);
  auto true_expr = this->stack.pop();

  ternary->false_expr->accept(this);
  auto false_expr = this->stack.pop();

  // May need to make this a tagged expression
  this->stack.push(BinaryenIf(this->mod, condition.value, true_expr.value,
                              false_expr.value));
}