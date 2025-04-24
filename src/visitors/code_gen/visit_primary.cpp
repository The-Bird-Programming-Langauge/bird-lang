#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

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

    this->stack.push(this->generate_string_from_string(str_value));
    break;
  }

  case Token::Type::CHAR_LITERAL: {
    const BinaryenExpressionRef c = BinaryenConst(
        this->mod, BinaryenLiteralInt32(primary->value.lexeme[0]));
    this->stack.push(TaggedExpression(c, std::make_shared<CharType>()));
    break;
  }

  case Token::Type::IDENTIFIER: {
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

TaggedExpression
CodeGen::generate_string_from_exprs(std::vector<BinaryenExpressionRef> vals) {
  unsigned int mem_size =
      vals.size() * bird_type_byte_size(std::make_shared<CharType>());
  std::shared_ptr<BirdType> type = std::make_shared<CharType>();
  auto &locals = this->function_locals[this->current_function_name];
  locals.push_back(BinaryenTypeInt32());

  auto identifier = std::to_string(locals.size() - 1) + "temp";
  this->environment.declare(
      identifier,
      TaggedIndex(locals.size() - 1, std::make_shared<StringType>()));

  std::vector<BinaryenExpressionRef> args = {
      BinaryenConst(this->mod, BinaryenLiteralInt32(mem_size)),
      BinaryenConst(this->mod, BinaryenLiteralInt32(0))};

  TaggedExpression local_set = this->binaryen_set(
      identifier, BinaryenCall(this->mod, "mem_alloc", args.data(), args.size(),
                               BinaryenTypeInt32()));

  std::vector<BinaryenExpressionRef> binaryen_calls = {local_set.value};

  unsigned int offset = 0;
  for (auto val : vals) {
    BinaryenExpressionRef args[3] = {
        this->binaryen_get(identifier).value,
        BinaryenConst(this->mod, BinaryenLiteralInt32(offset)), val};
    binaryen_calls.push_back(BinaryenCall(this->mod,
                                          get_mem_set_for_type(type->get_tag()),
                                          args, 3, BinaryenTypeNone()));
    offset += bird_type_byte_size(type);
  }

  std::vector<BinaryenExpressionRef> array_struct_args = {
      this->binaryen_get(identifier).value,
      BinaryenConst(this->mod, BinaryenLiteralInt32(vals.size()))};

  auto call = BinaryenCall(this->mod, struct_constructors["0array"].c_str(),
                           array_struct_args.data(), array_struct_args.size(),
                           BinaryenTypeInt32());

  auto create_ref = BinaryenCall(this->mod, struct_constructors["0ref"].c_str(),
                                 &call, 1, BinaryenTypeInt32());
  binaryen_calls.push_back(this->binaryen_set(identifier, create_ref).value);
  auto get_ref = this->binaryen_get(identifier);
  auto register_root = BinaryenCall(this->mod, "register_root", &get_ref.value,
                                    1, BinaryenTypeNone());

  binaryen_calls.push_back(register_root);
  binaryen_calls.push_back(this->binaryen_get(identifier).value);
  auto block = BinaryenBlock(this->mod, nullptr, binaryen_calls.data(),
                             binaryen_calls.size(), BinaryenTypeInt32());

  return TaggedExpression(block, std::make_shared<StringType>());
}

TaggedExpression CodeGen::generate_string_from_string(std::string string) {
  std::vector<BinaryenExpressionRef> vals;

  for (auto &element : string) {
    vals.push_back(BinaryenConst(this->mod, BinaryenLiteralInt32(element)));
  }

  return this->generate_string_from_exprs(vals);
}