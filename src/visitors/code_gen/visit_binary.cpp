#include "../../../include/visitors/code_gen.h"

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
  try {
    auto binary_op = this->binary_operations.at(binary->op.token_type);
    auto binary_op_fn = binary_op.at({left.type->type, right.type->type});

    this->stack.push(
        TaggedExpression(BinaryenBinary(this->mod, binary_op_fn.value(),
                                        left.value, right.value),
                         binary_op_fn.type));
  } catch (std::out_of_range &e) {
    throw BirdException("unsupported binary operation: " + binary->op.lexeme);
  }
}
