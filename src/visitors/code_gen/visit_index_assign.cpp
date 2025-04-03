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

  bool float_flag = (lhs_val.type->get_tag() == TypeTag::FLOAT &&
                     rhs_val.type->get_tag() == TypeTag::FLOAT);

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

  if (type_is_on_heap(rhs_val.type->get_tag())) {
    std::vector<BinaryenExpressionRef> get_operands = {
        result, BinaryenConst(this->mod, BinaryenLiteralInt32(0))};
    auto get_data = BinaryenCall(this->mod, "mem_get_32", get_operands.data(),
                                 get_operands.size(), BinaryenTypeInt32());
    std::vector<BinaryenExpressionRef> set_operands = {
        lhs_val.value, mem_offset_literal, get_data};
    auto set_data = BinaryenCall(this->mod, "mem_set_32", set_operands.data(),
                                 set_operands.size(), BinaryenTypeNone());

    this->stack.push(TaggedExpression(set_data, rhs_val.type));
    return;
  }

  auto array = this->deref(lhs.value);
  BinaryenExpressionRef args[3] = {this->get_array_data(array),
                                   mem_offset_literal, result};

  this->stack.push(BinaryenCall(this->mod,
                                get_mem_set_for_type(lhs_val.type->get_tag()),
                                args, 3, BinaryenTypeNone()));
}