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

  if (type_is_on_heap(result.type->get_tag())) {
    std::vector<BinaryenExpressionRef> get_operands = {
        result.value, BinaryenConst(this->mod, BinaryenLiteralInt32(0))};
    auto get_data = BinaryenCall(this->mod, "mem_get_32", get_operands.data(),
                                 get_operands.size(), BinaryenTypeInt32());
    std::vector<BinaryenExpressionRef> set_operands = {
        lhs_val.value, BinaryenConst(this->mod, BinaryenLiteralInt32(0)),
        get_data};
    auto set_data = BinaryenCall(this->mod, "mem_set_32", set_operands.data(),
                                 set_operands.size(), BinaryenTypeNone());

    this->stack.push(TaggedExpression(set_data, result.type));
    return;
  }

  TaggedExpression assign_stmt =
      this->binaryen_set(assign_expr->identifier.lexeme, result.value);

  this->stack.push(TaggedExpression(assign_stmt.value, result.type));
}
