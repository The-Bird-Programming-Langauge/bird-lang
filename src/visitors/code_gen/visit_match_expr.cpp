#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_match_expr(MatchExpr *match_expr) {
  match_expr->expr->accept(this);
  auto expr = this->stack.pop();

  this->stack.push(this->match_helper(expr, match_expr, 0));
}

TaggedExpression CodeGen::match_helper(TaggedExpression expr,
                                       MatchExpr *match_expr, int index) {
  if (index == match_expr->arms.size()) {
    match_expr->else_arm->accept(this);
    return this->stack.pop();
  }

  match_expr->arms[index].first->accept(this);
  const auto rhs = this->stack.pop();

  match_expr->arms[index].second->accept(this);
  const auto matched = this->stack.pop();

  BinaryenExpressionRef condition;
  if (rhs.type->type == BirdTypeType::STRING &&
      expr.type->type == BirdTypeType::STRING) {
    this->handle_binary_string_operations(Token::EQUAL_EQUAL, expr, rhs);
    condition = this->stack.pop().value;
  } else {
    auto equal_comparison =
        this->binary_operations.at(Token::Type::EQUAL_EQUAL);
    auto comparison_fn = equal_comparison.at({expr.type->type, rhs.type->type});
    condition =
        BinaryenBinary(this->mod, comparison_fn.value(), expr.value, rhs.value);
  }

  const auto if_false = match_helper(expr, match_expr, index + 1);
  return TaggedExpression(
      BinaryenIf(this->mod, condition, matched.value, if_false.value),
      if_false.type);
}