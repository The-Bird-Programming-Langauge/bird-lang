#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_assign_expr(AssignExpr *assign_expr) {
  TaggedIndex index = this->environment.get(assign_expr->identifier.lexeme);

  auto lhs_val = this->binaryen_get(assign_expr->identifier.lexeme);

  assign_expr->value->accept(this);
  TaggedExpression rhs_val = this->stack.pop();

  bool float_flag = (index.type->get_tag() == TypeTag::FLOAT &&
                     rhs_val.type->get_tag() == TypeTag::FLOAT);

  BinaryenExpressionRef result;
  switch (assign_expr->assign_operator.token_type) {
  case Token::Type::EQUAL: {
    result = rhs_val.value;
    break;
  }
  case Token::Type::PLUS_EQUAL: {
    // TODO: figure out string conatenation
    result = (float_flag) ? BinaryenBinary(this->mod, BinaryenAddFloat64(),
                                           lhs_val, rhs_val.value)
                          : BinaryenBinary(this->mod, BinaryenAddInt32(),
                                           lhs_val, rhs_val.value);

    break;
  }
  case Token::Type::MINUS_EQUAL: {
    result = (float_flag) ? BinaryenBinary(this->mod, BinaryenSubFloat64(),
                                           lhs_val, rhs_val.value)
                          : BinaryenBinary(this->mod, BinaryenSubInt32(),
                                           lhs_val, rhs_val.value);

    break;
  }
  case Token::Type::STAR_EQUAL: {
    result = (float_flag) ? BinaryenBinary(this->mod, BinaryenMulFloat64(),
                                           lhs_val, rhs_val.value)
                          : BinaryenBinary(this->mod, BinaryenMulInt32(),
                                           lhs_val, rhs_val.value);

    break;
  }
  case Token::Type::SLASH_EQUAL: {
    result = (float_flag) ? BinaryenBinary(this->mod, BinaryenDivFloat64(),
                                           lhs_val, rhs_val.value)
                          : BinaryenBinary(this->mod, BinaryenDivSInt32(),
                                           lhs_val, rhs_val.value);

    break;
  }
  case Token::Type::PERCENT_EQUAL: {
    result =
        (float_flag)
            ? throw BirdException("Modular operation requires integer values")
            : BinaryenBinary(this->mod, BinaryenRemSInt32(), lhs_val,
                             rhs_val.value);

    break;
  }
  default:
    throw BirdException("Unidentified assignment operator " +
                        assign_expr->assign_operator.lexeme);
    break;
  }

  BinaryenExpressionRef assign_stmt =
      this->binaryen_set(assign_expr->identifier.lexeme, result);

  this->stack.push(TaggedExpression(assign_stmt));
}
