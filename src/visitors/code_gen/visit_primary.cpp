#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_primary(Primary *primary) {
  switch (primary->value.token_type) {
  case Token::Type::INT_LITERAL: {
    int value = std::stoi(primary->value.lexeme);
    BinaryenExpressionRef int_literal =
        BinaryenConst(this->mod, BinaryenLiteralInt32(value));
    this->stack.push(TaggedExpression(
        int_literal, std::shared_ptr<BirdType>(new IntType())));
    break;
  }

  case Token::Type::FLOAT_LITERAL: {
    double value = std::stod(primary->value.lexeme);
    BinaryenExpressionRef float_literal =
        BinaryenConst(this->mod, BinaryenLiteralFloat64(value));
    this->stack.push(TaggedExpression(
        float_literal, std::shared_ptr<BirdType>(new FloatType())));
    break;
  }

  case Token::Type::TRUE: {
    BinaryenExpressionRef bool_literal =
        BinaryenConst(this->mod, BinaryenLiteralInt32(1));

    this->stack.push(TaggedExpression(
        bool_literal, std::shared_ptr<BirdType>(new BoolType())));
    break;
  }

  case Token::Type::FALSE: {
    BinaryenExpressionRef bool_literal =
        BinaryenConst(this->mod, BinaryenLiteralInt32(0));

    this->stack.push(TaggedExpression(
        bool_literal, std::shared_ptr<BirdType>(new BoolType())));
    break;
  }

  case Token::Type::STR_LITERAL: {
    const std::string &str_value = primary->value.lexeme;

    if (this->str_offsets.find(str_value) == this->str_offsets.end()) {
      throw BirdException("string not found: " + str_value);
    }

    BinaryenExpressionRef str_ptr = BinaryenConst(
        this->mod, BinaryenLiteralInt32(this->str_offsets[str_value]));
    this->stack.push(
        TaggedExpression(str_ptr, std::shared_ptr<BirdType>(new StringType())));
    break;
  }

  case Token::Type::IDENTIFIER: {
    if (auto fun =
            BinaryenGetFunction(this->mod, primary->value.lexeme.c_str())) {
      this->stack.push((BinaryenExpressionRef)fun);
      return;
    }
    // if in lambda, get from linear memory, and return
    std::cout << "finding: " << primary->value.lexeme
              << " in func: " << this->current_function_name << std::endl;
    if (primary->get_captured()) {
      std::cout << this->current_function_name << " locals: "
                << this->function_locals[this->current_function_name].size()
                << std::endl;
      const auto env = BinaryenLocalGet(this->mod, 0, BinaryenTypeInt32());

      std::vector<BinaryenExpressionRef> operands = {
          env, BinaryenConst(this->mod, BinaryenLiteralInt32(0))};
      const auto call = BinaryenCall(this->mod, "mem_get_32", operands.data(),
                                     operands.size(), BinaryenTypeInt32());

      this->stack.push(TaggedExpression(call, std::make_shared<IntType>()));
      return;
    }

    // how do we know where in linear memory?
    // we could reuse self?
    // use a map from the lambda name to the captured variables

    // else
    this->stack.push(this->binaryen_get(primary->value.lexeme));
    break;
  }

  case Token::Type::SELF: {
    this->stack.push(this->binaryen_get("self"));
    break;
  }

  default:
    throw BirdException("undefined primary value: " + primary->value.lexeme);
  }
}