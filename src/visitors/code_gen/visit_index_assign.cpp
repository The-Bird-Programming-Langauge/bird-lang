#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_index_assign(IndexAssign *index_assign) {
  index_assign->lhs->subscriptable->accept(this);
  auto lhs = this->stack.pop();

  index_assign->lhs->accept(this);
  auto lhs_val = this->stack.pop();

  index_assign->lhs->index->accept(this);
  auto index = this->stack.pop();

  index_assign->rhs->accept(this);
  auto rhs_val = this->stack.pop();

  auto index_literal = BinaryenBinary(
      this->mod, BinaryenMulInt32(), index.value,
      BinaryenConst(this->mod,
                    BinaryenLiteralInt32(bird_type_byte_size(lhs_val.type))));

  bool float_flag = (lhs_val.type->type == BirdTypeType::FLOAT &&
                     rhs_val.type->type == BirdTypeType::FLOAT);

  BinaryenExpressionRef result;
  switch (index_assign->op.token_type) {
  case Token::Type::EQUAL: {
    result = rhs_val.value;

    break;
  }
  case Token::Type::PLUS_EQUAL: {
    result = (float_flag) ? BinaryenBinary(this->mod, BinaryenAddFloat64(),
                                           lhs_val.value, rhs_val.value)
                          : BinaryenBinary(this->mod, BinaryenAddInt32(),
                                           lhs_val.value, rhs_val.value);
    break;
  }
  case Token::Type::MINUS_EQUAL: {
    result = (float_flag) ? BinaryenBinary(this->mod, BinaryenSubFloat64(),
                                           lhs_val.value, rhs_val.value)
                          : BinaryenBinary(this->mod, BinaryenSubInt32(),
                                           lhs_val.value, rhs_val.value);

    break;
  }
  case Token::Type::STAR_EQUAL: {
    result = (float_flag) ? BinaryenBinary(this->mod, BinaryenMulFloat64(),
                                           lhs_val.value, rhs_val.value)
                          : BinaryenBinary(this->mod, BinaryenMulInt32(),
                                           lhs_val.value, rhs_val.value);

    break;
  }
  case Token::Type::SLASH_EQUAL: {
    result = (float_flag) ? BinaryenBinary(this->mod, BinaryenDivFloat64(),
                                           lhs_val.value, rhs_val.value)
                          : BinaryenBinary(this->mod, BinaryenDivSInt32(),
                                           lhs_val.value, rhs_val.value);

    break;
  }
  case Token::Type::PERCENT_EQUAL: {
    result =
        (float_flag)
            ? throw BirdException("Modular operation requires integer values")
            : BinaryenBinary(this->mod, BinaryenRemSInt32(), lhs_val.value,
                             rhs_val.value);

    break;
  }
  default:
    throw BirdException("Unidentified assignment operator " +
                        index_assign->op.lexeme);
    break;
  }

  BinaryenExpressionRef args[3] = {lhs.value, index_literal, result};

  this->stack.push(BinaryenCall(this->mod,
                                get_mem_set_for_type(lhs_val.type->type), args,
                                3, BinaryenTypeNone()));
}