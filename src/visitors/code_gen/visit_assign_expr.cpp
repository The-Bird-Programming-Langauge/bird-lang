#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_assign_expr(AssignExpr *assign_expr) {
  TaggedIndex index = this->environment.get(assign_expr->identifier.lexeme);

  auto lhs_val = this->binaryen_get(assign_expr->identifier.lexeme);

  assign_expr->value->accept(this);
  TaggedExpression rhs_val = this->stack.pop();

  TaggedExpression result = rhs_val;
  if (assign_expr->assign_operator.token_type != Token::Type::EQUAL) {
    visit_binary_normal(
        assign_expr_binary_equivalent(assign_expr->assign_operator.token_type),
        lhs_val, rhs_val);
    result = stack.pop();
  }

  TaggedExpression assign_stmt = this->binaryen_set(
      this->name_mangler + assign_expr->identifier.lexeme, result.value);

  this->stack.push(TaggedExpression(assign_stmt.value, result.type));
}
