#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_assign_expr(AssignExpr *assign_expr) {
  TaggedIndex index = this->environment.get(assign_expr->identifier.lexeme);

  auto lhs_val = this->binaryen_get(assign_expr->identifier.lexeme);

  assign_expr->value->accept(this);
  TaggedExpression rhs_val = this->stack.pop();

  BinaryenExpressionRef result;

  if (assign_expr->assign_operator.token_type == Token::Type::EQUAL) {
    result = rhs_val.value;
  } else {

    // TODO: refactor binary and assign types to hold an operation type
    // instead of a token type to avoid repetition of this sort of thing
    Token::Type assign_op_token_as_binary_op_token;
    switch (assign_expr->assign_operator.token_type) {
    case Token::Type::PLUS_EQUAL:
      assign_op_token_as_binary_op_token = Token::Type::PLUS;
      break;
    case Token::Type::MINUS_EQUAL:
      assign_op_token_as_binary_op_token = Token::Type::MINUS;
      break;
    case Token::Type::STAR_EQUAL:
      assign_op_token_as_binary_op_token = Token::Type::STAR;
      break;
    case Token::Type::SLASH_EQUAL:
      assign_op_token_as_binary_op_token = Token::Type::SLASH;
      break;
    case Token::Type::PERCENT_EQUAL:
      assign_op_token_as_binary_op_token = Token::Type::PERCENT;
      break;
    default:
      throw BirdException("Unidentified assignment operator " +
                          assign_expr->assign_operator.lexeme);
      break;
    }

    try {
      auto binary_op =
          this->binary_operations.at(assign_op_token_as_binary_op_token);
      auto binary_op_fn =
          binary_op.at({index.type->get_tag(), rhs_val.type->get_tag()});

      result = BinaryenBinary(this->mod, binary_op_fn.value(), lhs_val,
                              rhs_val.value);
    } catch (std::out_of_range &e) {
      throw BirdException("unsupported assign operation: " +
                          assign_expr->assign_operator.lexeme + " on types " +
                          index.type->to_string() + " and " +
                          rhs_val.type->to_string());
    }
  }

  BinaryenExpressionRef assign_stmt =
      this->binaryen_set(assign_expr->identifier.lexeme, result);

  this->stack.push(TaggedExpression(assign_stmt));
}
