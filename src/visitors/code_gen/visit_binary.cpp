#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

void CodeGen::visit_binary(Binary *binary) {
  switch (binary->op.token_type) {
  case Token::Type::AND:
  case Token::Type::OR:
    visit_binary_short_circuit(binary);
    break;
  default:
    visit_binary_normal(binary);
  }
}

void CodeGen::visit_binary_short_circuit(Binary *binary) {
  binary->left->accept(this);
  binary->right->accept(this);

  auto right = this->stack.pop();
  auto left = this->stack.pop();
  switch (binary->op.token_type) {
  case Token::Type::AND: {
    this->stack.push(TaggedExpression(
        BinaryenIf(this->mod,
                   BinaryenUnary(this->mod, BinaryenEqZInt32(), left.value),
                   left.value, right.value),
        std::shared_ptr<BirdType>(new BoolType())));
    break;
  }
  case Token::Type::OR: {
    this->stack.push(TaggedExpression(
        BinaryenIf(this->mod,
                   BinaryenUnary(this->mod, BinaryenEqZInt32(), left.value),
                   right.value, left.value),
        std::shared_ptr<BirdType>(new BoolType())));
    break;
  }
  default:
    throw BirdException("unsupported short circuit operation: " +
                        binary->op.lexeme);
  }
}

void CodeGen::visit_binary_normal(Binary *binary) {
  binary->left->accept(this);
  binary->right->accept(this);

  auto right = this->stack.pop();
  auto left = this->stack.pop();

  if (right.type->get_tag() == TypeTag::STRING &&
      left.type->get_tag() == TypeTag::STRING) {
    this->handle_binary_string_operations(binary->op.token_type, right, left);
    return;
  }

  try {
    auto binary_op = this->binary_operations.at(binary->op.token_type);
    auto binary_op_fn =
        binary_op.at({left.type->get_tag(), right.type->get_tag()});

    this->stack.push(
        TaggedExpression(BinaryenBinary(this->mod, binary_op_fn.value(),
                                        left.value, right.value),
                         binary_op_fn.type));
  } catch (std::out_of_range &e) {
    throw BirdException("unsupported binary operation: " + binary->op.lexeme);
  }
}

void CodeGen::handle_binary_string_operations(Token::Type op,
                                              TaggedExpression left,
                                              TaggedExpression right) {
  BinaryenExpressionRef operands[2] = {left.value, right.value};
  switch (op) {
  case Token::PLUS: {
    this->stack.push(TaggedExpression(
        BinaryenCall(this->mod, "strcat", operands, 2, BinaryenTypeInt32()),
        std::make_shared<StringType>(true)));
    break;
  }
  case Token::EQUAL_EQUAL: {
    this->stack.push(TaggedExpression(
        BinaryenCall(this->mod, "strcmp", operands, 2, BinaryenTypeInt32()),
        std::make_shared<BoolType>()));
    break;
  }
  case Token::BANG_EQUAL: {
    this->stack.push(this->create_unary_not(
        BinaryenCall(this->mod, "strcmp", operands, 2, BinaryenTypeInt32())));
    break;
  }
  default:
    throw BirdException("invalid binary string operation");
  }
}