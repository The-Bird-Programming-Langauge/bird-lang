#include "../../../include/visitors/code_gen.h"
#include <memory>

void CodeGen::visit_index_assign(IndexAssign *index_assign) {
  index_assign->lhs->subscriptable->accept(this);
  auto lhs = this->stack.pop();

  index_assign->lhs->accept(this);
  auto lhs_val = this->stack.pop();

  index_assign->lhs->index->accept(this);
  auto index = this->stack.pop();

  index_assign->rhs->accept(this);
  auto rhs_val = this->stack.pop();

  auto mem_offset_literal = BinaryenBinary(
      this->mod, BinaryenMulInt32(), index.value,
      BinaryenConst(this->mod,
                    BinaryenLiteralInt32(bird_type_byte_size(lhs_val.type))));

  BinaryenExpressionRef result;

  if (index_assign->op.token_type == Token::Type::EQUAL) {
    result = rhs_val.value;
  } else {

    // TODO: refactor binary and assign types to hold an operation type
    // instead of a token type to avoid repetition of this sort of thing
    Token::Type assign_op_token_as_binary_op_token;
    switch (index_assign->op.token_type) {
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
                          index_assign->op.lexeme);
      break;
    }

    try {
      auto binary_op =
          this->binary_operations.at(assign_op_token_as_binary_op_token);
      auto binary_op_fn =
          binary_op.at({lhs_val.type->get_tag(), rhs_val.type->get_tag()});

      result = BinaryenBinary(this->mod, binary_op_fn.value(), lhs_val.value,
                              rhs_val.value);
    } catch (std::out_of_range &e) {
      throw BirdException(
          "unsupported assign operation: " + index_assign->op.lexeme +
          " on types " + lhs_val.type->to_string() + " and " +
          rhs_val.type->to_string());
    }
  }

  BinaryenExpressionRef args[3] = {this->get_array_data(lhs),
                                   mem_offset_literal, result};

  this->stack.push(BinaryenCall(this->mod,
                                get_mem_set_for_type(lhs_val.type->get_tag()),
                                args, 3, BinaryenTypeNone()));
}