#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_for_stmt(ForStmt *for_stmt) {
  this->environment.push_env();
  std::vector<BinaryenExpressionRef> children;

  TaggedExpression initializer;
  if (for_stmt->initializer.has_value()) {
    for_stmt->initializer.value()->accept(this);
    initializer = this->stack.pop();
  }

  TaggedExpression condition;
  if (for_stmt->condition.has_value()) {
    for_stmt->condition.value()->accept(this);
    condition = this->stack.pop();
  }

  for_stmt->body->accept(this);
  TaggedExpression body = this->stack.pop();

  children.push_back(body.value);

  TaggedExpression increment;
  if (for_stmt->increment.has_value()) {
    for_stmt->increment.value()->accept(this);
    increment = this->stack.pop();
  }

  auto body_and_condition = BinaryenBlock(this->mod, "BODY", children.data(),
                                          children.size(), BinaryenTypeNone());

  std::vector<BinaryenExpressionRef> body_and_increment_children;
  body_and_increment_children.push_back(body_and_condition);

  if (increment.value) {
    body_and_increment_children.push_back(increment.value);
  }

  if (condition.value) {
    body_and_increment_children.push_back(
        BinaryenBreak(this->mod, "LOOP", condition.value, nullptr));
  } else {
    body_and_increment_children.push_back(
        BinaryenBreak(this->mod, "LOOP", nullptr, nullptr));
  }

  auto body_and_increment =
      BinaryenBlock(this->mod, "for_body", body_and_increment_children.data(),
                    body_and_increment_children.size(), BinaryenTypeNone());

  auto for_loop = BinaryenLoop(this->mod, "LOOP", body_and_increment);

  std::vector<BinaryenExpressionRef> initializer_and_loop;
  if (initializer.value) {
    initializer_and_loop.push_back(initializer.value);
  }

  BinaryenExpressionRef block;
  if (condition.value) {
    block = BinaryenIf(this->mod, condition.value, for_loop,
                       initializer.value
                           ? BinaryenDrop(this->mod, initializer.value)
                           : nullptr);
  } else {
    block = for_loop;
  }

  initializer_and_loop.push_back(block);

  auto exit_block =
      BinaryenBlock(this->mod, "EXIT", initializer_and_loop.data(),
                    initializer_and_loop.size(), BinaryenTypeNone());

  this->stack.push(exit_block);

  this->environment.pop_env();
}
